#pragma comment(lib, "libprotobufd.lib")

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <boost/asio.hpp>

#include "protocol.pb.h"
#include "Game_server.h"
#include "PB_handler.h"

using namespace std;

int main(int argc, char **argv)
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	boost::asio::io_service io_service;

	dna_info::LoginRequest login_req;
	login_req.set_id("kernelgarden");
	login_req.set_passwd("sunrin123");

	dna_info::LoginResponse login_res;
	login_res.set_response_code(200);

	dna_info::PacketHeader_PB header1;
	header1.set_packet_size(100000000);
	header1.set_packet_type(100000000);

	dna_info::PacketHeader_PB header2;
	header2.set_packet_size(1000);
	header2.set_packet_type(1);

	std::cout << "[" << header1.ByteSize() << ", " << header2.ByteSize() << "]" << std::endl;

	int buf_size = 0;
	buf_size += sizeof(PacketHeader) + login_req.ByteSize();
	buf_size += sizeof(PacketHeader) + login_res.ByteSize();
	protobuf::uint8 *outputBuf = new protobuf::uint8[buf_size];

	protobuf::io::ArrayOutputStream output_array_stream(outputBuf, buf_size);
	protobuf::io::CodedOutputStream output_coded_stream(&output_array_stream);

	WriteMessageToStream(login_req, dna_info::LOGIN_REQ, output_coded_stream);
	WriteMessageToStream(login_res, dna_info::LOGIN_RES, output_coded_stream);

	PacketHandler handler;
	protobuf::io::ArrayInputStream input_array_stream(outputBuf, buf_size);
	protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

	Process_packet(input_coded_stream, handler);

	GameServer server(io_service);

	server.Init();
	server.Start();

	io_service.run();

	delete[]outputBuf;
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}