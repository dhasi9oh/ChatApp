#include "CSession.h"
#include "CServer.h"
#include <iostream>
#include <sstream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include "LogicSystem.h"

CSession::CSession(boost::asio::io_context& io_context, CServer* server):
	_socket(io_context), _server(server), _b_close(false),_b_head_parse(false){
	boost::uuids::uuid  a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	_recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}
CSession::~CSession() {
    LOG_INFO("~CSession destruct");
}

tcp::socket& CSession::GetSocket() {
	return _socket;
}

std::string& CSession::GetUuid() {
	return _uuid;
}

void CSession::Start(){
	AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::Send(std::string msg, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
        LOG_WARN("session: {}  send que fulled, size is {}", _uuid, MAX_SENDQUE);
		return;
	}

	_send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Send(char* msg, short max_length, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
        LOG_WARN("session: {} send que fulled, size is {}", _uuid, MAX_SENDQUE);
		return;
	}

	_send_que.push(make_shared<SendNode>(msg, max_length, msgid));
	if (send_que_size>0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len), 
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Close() {
	_socket.close();
	_b_close = true;
}

std::shared_ptr<CSession>CSession::SharedSelf() {
	return shared_from_this();
}

void CSession::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
                LOG_WARN("handle read failed, error is {}", ec.message());
				Close();
				_server->ClearSession(_uuid);
				return;
			}

            if (bytes_transfered < total_len) {
                LOG_WARN("read length not match, read [{}] , total [{}]", bytes_transfered, total_len);
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			memcpy(_recv_msg_node->_data , _data , bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
            _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            LOG_INFO("receive data is {}", _recv_msg_node->_data);
			//�˴�����ϢͶ�ݵ��߼�������
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//��������ͷ�������¼�
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
            LOG_ERR("Exception code is {}", e.what());
		}
		});
}

void CSession::AsyncReadHead(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
            if (ec) {
                LOG_WARN("handle read failed, error is {}", ec.message());
				Close();
				_server->ClearSession(_uuid);
				return;
			}

            if (bytes_transfered < HEAD_TOTAL_LEN) {
                LOG_WARN("read length not match, read [{}] , total [{}]", bytes_transfered, HEAD_TOTAL_LEN);
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered);

			//��ȡͷ��MSGID����
			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
			//�����ֽ���ת��Ϊ�����ֽ���
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
            LOG_INFO("msg_id  is {}", msg_id);
			//id�Ƿ�
			if (msg_id > MAX_LENGTH) {
                LOG_WARN("invalid msg_id is {}", msg_id);
				_server->ClearSession(_uuid);
				return;
			}
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			//�����ֽ���ת��Ϊ�����ֽ���
            msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
            LOG_INFO("msg_len is {}", msg_len);

			//id�Ƿ�
            if (msg_len > MAX_LENGTH) {
                LOG_WARN("invalid data length is {}", msg_len);
				_server->ClearSession(_uuid);
				return;
			}

			_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
			AsyncReadBody(msg_len);
		}
        catch (std::exception& e) {
            LOG_ERR("Exception code is {}", e.what());
		}
		});
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self) {
	//�����쳣����
	try {
		if (!error) {
            std::lock_guard<std::mutex> lock(_send_lock);
			_send_que.pop();
			if (!_send_que.empty()) {
				auto& msgnode = _send_que.front();
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
            LOG_WARN("handle write failed, error is {}", error.message());
			Close();
			_server->ClearSession(_uuid);
		}
	}
	catch (std::exception& e) {
        LOG_ERR("Exception code : ", e.what());
	}
	
}

//��ȡ��������
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler )
{
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

//��ȡָ���ֽ���
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len, 
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len-read_len),
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
			if (ec) {
				// ���ִ��󣬵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				//���ȹ��˾͵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// û�д����ҳ��Ȳ����������ȡ
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
	});
}

LogicNode::LogicNode(shared_ptr<CSession>  session, 
	shared_ptr<RecvNode> recvnode):_session(session),_recvnode(recvnode) {
	
}