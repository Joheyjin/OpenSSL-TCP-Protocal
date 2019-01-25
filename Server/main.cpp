#include <iostream>
#include <winsock.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

//�ٽ� ������ ������ä, �� ���� SSL�� �ٿ��� ���

void init()
{
	WSADATA wsaData; // ���Ӱ�ü�� �ʱ�ȭ
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SSL_load_error_strings(); // SSL�̿��Ͽ� SSL_library�Լ����� �ʱ�ȭ�� �� ����
	SSL_library_init(); 
	OpenSSL_add_all_algorithms(); // ���ú��Լ��� ������ ���� ���̺귯���� �ƴ�, OpenSSL���̺귯���� �̿��� // �����Լ����� ���������� OpenSLL�� �̿� �Ѵٴ� ���� ���� �հ��ǿ� �Ȱ�(�⺻���� ���۰����� �Ȱ�)
}

void close()
{
	ERR_free_strings(); // OpenSSL�� ���� cleanup�Լ�
	EVP_cleanup(); // "
	WSACleanup();
}

int main()
{	//  winsock�� ���� ������ �Ȱ���
	init();
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddress;
	int addressLength = sizeof(serverAddress);

	memset((char *)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(9876);

	bind(sockfd, (struct sockaddr *) &serverAddress, addressLength);

	listen(sockfd, 10);

	/* SSL ��ü �ʱ�ȭ */
	SSL_CTX *sslContext = SSL_CTX_new(SSLv23_server_method());
	SSL_CTX_set_options(sslContext, SSL_OP_SINGLE_DH_USE);

	/* ����Ű�� ����Ű �ʱ�ȭ */ // OpenSLL�� CTX�� �̿��� ����Ű�� ����Ű�� �ʱ�ȭ �� �� ����, �����Լ��� ���� OpenSLL ���̺귯���� �ʱ�ȭ�Ѵ�
	SSL_CTX_use_certificate_file(sslContext, "./cert.pem", SSL_FILETYPE_PEM); // ����Ű
	SSL_CTX_use_PrivateKey_file(sslContext, "./key.pem", SSL_FILETYPE_PEM); // ����Ű

	while (true)
	{
		int fd = accept(sockfd, (struct sockaddr *) &serverAddress, &addressLength); // bind->listen->accept�� �� ���� Ŭ���̾�Ʈ�� ������ �õ�

		/* SSL ��� ó�� */
		SSL *ssl = SSL_new(sslContext);
		SSL_set_fd(ssl, fd);
		SSL_accept(ssl); // Ŭ���̾�Ʈ�� ������ ����(accept�Լ��̿�)�ϴ� ������ �־, ���� ������ ����Ű�� ����Ű�� ����ϰԵ�

		/* SSL �Է� */
		char input[4096] = { 0 };
		SSL_read(ssl, (char *)input, 4096); // winsock�� �ٸ��� �ΰ����� �÷��װ��� ���� ���� // ���ڿ����ۿ� 4096��ŭ�� �����͸� �ްڴ�

		/* SSL ��� */
		char output[4096] = { 0 };
		int length = wsprintfA(output, "[Echo]: %s\n", input); // output�� ����ڰ� �Է��� input���� �״�� �־� ����ڿ��� echo���� ������
		SSL_write(ssl, output, length);
		SSL_free(ssl);
		closesocket(fd);
	} // ���ڰ��� �����ִ� ����
	SSL_CTX_free(sslContext); // ���� SSL_CTX �� �޸𸮸� ����
	
	close();
	return 0;
}