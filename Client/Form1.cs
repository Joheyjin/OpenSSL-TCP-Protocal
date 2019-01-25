using System;
using System.Text;
using System.Windows.Forms;
using System.Net.Sockets; // 이거와
using System.Net.Security; // 이거와
using System.Security.Cryptography.X509Certificates; // 이것은 C#에서 Security와  Net 라이브러리 안에서 Socket과 OpenSLL의 암호화 통신을 지원함
namespace Client
{
    public partial class Form1 : Form
    {
        public string serverIP = "127.0.0.1";
        public int port = 9876;
        public string serverDomain = "localhost";
        public Form1()
        {
            InitializeComponent();
        }
        private void button_Click(object sender, EventArgs e)
        { // 기존의 통신 객체 ssl을 붙이는 방식으로,  OpenSSL을 입힐수 있음
            TcpClient client = new TcpClient(serverIP, port); 
            SslStream sslStream = new SslStream(client.GetStream(), false, validateCertificate, null); // 방금 만든 client객체에 stream값을 얻어와서 Ssl을 덮어씌울 수 있다
            sslStream.AuthenticateAsClient(serverDomain);

            byte[] buf = Encoding.ASCII.GetBytes("Hello SSL!"); // Hello SSL!이라는 문자열을 버퍼에 담아서  
            sslStream.Write(buf, 0, buf.Length); // 메시지를 전송하고, 
            sslStream.Flush(); // 전송이후에는, 곧바로 에코서버로 부터 메시지를 전달받아야 함으로 Flush()를 해줌

            buf = new byte[4096];
            int length = sslStream.Read(buf, 0, 4096); // sslStream 객체를 활용하여, 서버로부터 OpenSSL을 활용해서 데이터를 입력받음
            messageText.Text = Encoding.ASCII.GetString(buf, 0, length); // messageText의 값으로 에코서버로 부터 전달값을 그대로 출력하도록 만듦
        }
        private bool validateCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            return true;
        }
    }
} //OpenSSL을 적용하여, Client가 TCP통신을 하는 예제