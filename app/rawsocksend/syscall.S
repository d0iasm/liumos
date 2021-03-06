.intel_syntax noprefix

// Syscall numbers:
// https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md#x86_64-64_bit

// System V AMD64 ABI:
//   args: RDI, RSI, RDX, RCX, R8, R9
//   callee-saved: RBX, RBP, R12, R13, R14, R15
//   caller-saved: otherwise

// Linux kernel x86-64 syscall:
//   idx: RAX
//   args: RDI, RSI, RDX, R10, R8, R9

.global write
write:
	mov rax, 1
	syscall
	ret

// int socket(int domain, int type, int protocol);
.global socket
socket:
	// arg[1]: rdi = rdi
	// arg[2]: rsi = rsi
	// arg[3]: rdx = rdx
	mov rax, 41
	syscall
	ret

// ssize_t sendto(
//     int sockfd, const void *buf,
//     size_t len, int flags,
//     const struct sockaddr *dest_addr,
//     socklen_t addrlen);
.global sendto
sendto:
	mov rax, 44
	// arg[1]: rdi = rdi
	// arg[2]: rsi = rsi
	// arg[3]: rdx = rdx
	mov r10, rcx
	// arg[5]: r8 = r8
	// arg[5]: r9 = r9
	syscall
	ret

// int close(int fd);
.global close
close:
	mov rax, 3
	// arg[1]: rdi = rdi
	syscall
	ret

.global exit
exit:
	mov rax, 60
	syscall
