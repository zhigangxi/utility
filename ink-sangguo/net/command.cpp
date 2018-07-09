#include "command.h"
#include <google/protobuf/message.h>
#include <string>
using namespace std;

void CCommand::AddCommand(uint32_t type, CommandFun fun)
{
	CmdData data;
	data.fun = fun;
	m_funcTable.Insert(type, data);
}

void CCommand::RunCmd(UserSession session, uint32_t msgType,SharedMsg msg)
{
	/*google::protobuf::Message *message = msg.get();
	if (message == nullptr)
		return;

	string typeName = message->GetTypeName();
	static int len = strlen("NetMsg.");
	typeName.insert(len, "MessageType.");
	const google::protobuf::EnumValueDescriptor* enum_value_descriptor = google::protobuf::DescriptorPool::generated_pool()->FindEnumValueByName(typeName);
	if (!enum_value_descriptor)
	{
		cout << "recv invalid message:" << typeName.c_str() << endl;
		return;
	}
	uint32_t msgType = (uint32_t)enum_value_descriptor->number();*/

	CmdData *data = m_funcTable.Find(msgType);
	if(data == nullptr)
		return;

	data->fun(session, msg);
	data->runTimes++;
}