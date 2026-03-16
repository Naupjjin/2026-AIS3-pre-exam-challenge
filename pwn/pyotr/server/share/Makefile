all:
	CGO_CFLAGS="-fno-stack-protector -D_FORTIFY_SOURCE=0" CGO_LDFLAGS="-fno-stack-protector" go build -buildmode=pie -o chal chal.go
