#include <iostream>
#include <winsock.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

//핵심 로직은 유지한채, 그 위에 SSL을 붙여서 통신

void init()
{
	WSADATA wsaData; // 윈속객체를 초기화
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SSL_load_error_strings(); // SSL이용하여 SSL_library함수까지 초기화할 수 있음
	SSL_library_init(); 
	OpenSSL_add_all_algorithms(); // 리시브함수를 기존의 윈속 라이브러리가 아닌, OpenSSL라이브러리로 이용함 // 샌드함수또한 마찬가지로 OpenSLL을 이용 한다는 점만 빼면 앞강의와 똑같(기본적인 동작과정은 똑같)
}

void close()
{
	ERR_free_strings(); // OpenSSL을 위한 cleanup함수
	EVP_cleanup(); // "
	WSACleanup();
}

int main()
{	//  winsock의 앞의 예제와 똑같음
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

	/* SSL 객체 초기화 */
	SSL_CTX *sslContext = SSL_CTX_new(SSLv23_server_method());
	SSL_CTX_set_options(sslContext, SSL_OP_SINGLE_DH_USE);

	/* 공개키와 개인키 초기화 */ // OpenSLL은 CTX를 이용해 공개키와 개인키를 초기화 할 수 있음, 전역함수를 통해 OpenSLL 라이브러리를 초기화한다
	SSL_CTX_use_certificate_file(sslContext, "./cert.pem", SSL_FILETYPE_PEM); // 공개키
	SSL_CTX_use_PrivateKey_file(sslContext, "./key.pem", SSL_FILETYPE_PEM); // 개인키

	while (true)
	{
		int fd = accept(sockfd, (struct sockaddr *) &serverAddress, &addressLength); // bind->listen->accept를 한 이후 클라이언트와 연결을 시도

		/* SSL 통신 처리 */
		SSL *ssl = SSL_new(sslContext);
		SSL_set_fd(ssl, fd);
		SSL_accept(ssl); // 클라이언트와 연결을 수립(accept함수이용)하는 과정에 있어서, 위의 설정한 공개키와 개인키를 사용하게됨

		/* SSL 입력 */
		char input[4096] = { 0 };
		SSL_read(ssl, (char *)input, 4096); // winsock과 다르게 부가적인 플래그값을 넣진 않음 // 문자열버퍼에 4096만큼의 데이터를 받겠다

		/* SSL 출력 */
		char output[4096] = { 0 };
		int length = wsprintfA(output, "[Echo]: %s\n", input); // output에 사용자가 입력한 input값을 그대로 넣어 사용자에게 echo값을 돌려줌
		SSL_write(ssl, output, length);
		SSL_free(ssl);
		closesocket(fd);
	} // 에코값을 돌려주는 예제
	SSL_CTX_free(sslContext); // 위의 SSL_CTX 의 메모리를 해제
	
	close();
	return 0;
}