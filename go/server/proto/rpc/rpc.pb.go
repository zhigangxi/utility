// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.21.0
// 	protoc        v3.11.4
// source: rpc.proto

package rpcpb

import (
	proto "github.com/golang/protobuf/proto"
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

// This is a compile-time assertion that a sufficiently up-to-date version
// of the legacy proto package is being used.
const _ = proto.ProtoPackageIsVersion4

type PbType_TypeEnum int32

const (
	PbType_RpcTimer          PbType_TypeEnum = 0 //don't modify this
	PbType_GetLoginSessArg   PbType_TypeEnum = 1
	PbType_GetLoginSessReply PbType_TypeEnum = 2
	PbType_GameServerInfo    PbType_TypeEnum = 3
)

// Enum value maps for PbType_TypeEnum.
var (
	PbType_TypeEnum_name = map[int32]string{
		0: "RpcTimer",
		1: "GetLoginSessArg",
		2: "GetLoginSessReply",
		3: "GameServerInfo",
	}
	PbType_TypeEnum_value = map[string]int32{
		"RpcTimer":          0,
		"GetLoginSessArg":   1,
		"GetLoginSessReply": 2,
		"GameServerInfo":    3,
	}
)

func (x PbType_TypeEnum) Enum() *PbType_TypeEnum {
	p := new(PbType_TypeEnum)
	*p = x
	return p
}

func (x PbType_TypeEnum) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (PbType_TypeEnum) Descriptor() protoreflect.EnumDescriptor {
	return file_rpc_proto_enumTypes[0].Descriptor()
}

func (PbType_TypeEnum) Type() protoreflect.EnumType {
	return &file_rpc_proto_enumTypes[0]
}

func (x PbType_TypeEnum) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use PbType_TypeEnum.Descriptor instead.
func (PbType_TypeEnum) EnumDescriptor() ([]byte, []int) {
	return file_rpc_proto_rawDescGZIP(), []int{0, 0}
}

type PbType struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields
}

func (x *PbType) Reset() {
	*x = PbType{}
	if protoimpl.UnsafeEnabled {
		mi := &file_rpc_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *PbType) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*PbType) ProtoMessage() {}

func (x *PbType) ProtoReflect() protoreflect.Message {
	mi := &file_rpc_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use PbType.ProtoReflect.Descriptor instead.
func (*PbType) Descriptor() ([]byte, []int) {
	return file_rpc_proto_rawDescGZIP(), []int{0}
}

type GetLoginSessArg struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	UserId int64 `protobuf:"varint,1,opt,name=userId,proto3" json:"userId,omitempty"`
}

func (x *GetLoginSessArg) Reset() {
	*x = GetLoginSessArg{}
	if protoimpl.UnsafeEnabled {
		mi := &file_rpc_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *GetLoginSessArg) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*GetLoginSessArg) ProtoMessage() {}

func (x *GetLoginSessArg) ProtoReflect() protoreflect.Message {
	mi := &file_rpc_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use GetLoginSessArg.ProtoReflect.Descriptor instead.
func (*GetLoginSessArg) Descriptor() ([]byte, []int) {
	return file_rpc_proto_rawDescGZIP(), []int{1}
}

func (x *GetLoginSessArg) GetUserId() int64 {
	if x != nil {
		return x.UserId
	}
	return 0
}

type GetLoginSessReply struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	UserId  int64  `protobuf:"varint,1,opt,name=userId,proto3" json:"userId,omitempty"`
	Session string `protobuf:"bytes,2,opt,name=session,proto3" json:"session,omitempty"`
}

func (x *GetLoginSessReply) Reset() {
	*x = GetLoginSessReply{}
	if protoimpl.UnsafeEnabled {
		mi := &file_rpc_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *GetLoginSessReply) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*GetLoginSessReply) ProtoMessage() {}

func (x *GetLoginSessReply) ProtoReflect() protoreflect.Message {
	mi := &file_rpc_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use GetLoginSessReply.ProtoReflect.Descriptor instead.
func (*GetLoginSessReply) Descriptor() ([]byte, []int) {
	return file_rpc_proto_rawDescGZIP(), []int{2}
}

func (x *GetLoginSessReply) GetUserId() int64 {
	if x != nil {
		return x.UserId
	}
	return 0
}

func (x *GetLoginSessReply) GetSession() string {
	if x != nil {
		return x.Session
	}
	return ""
}

type GameServerInfo struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	ServerIp   string `protobuf:"bytes,1,opt,name=serverIp,proto3" json:"serverIp,omitempty"`
	ServerName string `protobuf:"bytes,2,opt,name=serverName,proto3" json:"serverName,omitempty"`
	OrderId    int32  `protobuf:"varint,3,opt,name=orderId,proto3" json:"orderId,omitempty"`
	UserNum    int32  `protobuf:"varint,4,opt,name=userNum,proto3" json:"userNum,omitempty"`
}

func (x *GameServerInfo) Reset() {
	*x = GameServerInfo{}
	if protoimpl.UnsafeEnabled {
		mi := &file_rpc_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *GameServerInfo) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*GameServerInfo) ProtoMessage() {}

func (x *GameServerInfo) ProtoReflect() protoreflect.Message {
	mi := &file_rpc_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use GameServerInfo.ProtoReflect.Descriptor instead.
func (*GameServerInfo) Descriptor() ([]byte, []int) {
	return file_rpc_proto_rawDescGZIP(), []int{3}
}

func (x *GameServerInfo) GetServerIp() string {
	if x != nil {
		return x.ServerIp
	}
	return ""
}

func (x *GameServerInfo) GetServerName() string {
	if x != nil {
		return x.ServerName
	}
	return ""
}

func (x *GameServerInfo) GetOrderId() int32 {
	if x != nil {
		return x.OrderId
	}
	return 0
}

func (x *GameServerInfo) GetUserNum() int32 {
	if x != nil {
		return x.UserNum
	}
	return 0
}

var File_rpc_proto protoreflect.FileDescriptor

var file_rpc_proto_rawDesc = []byte{
	0x0a, 0x09, 0x72, 0x70, 0x63, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x05, 0x72, 0x70, 0x63,
	0x70, 0x62, 0x22, 0x62, 0x0a, 0x06, 0x50, 0x62, 0x54, 0x79, 0x70, 0x65, 0x22, 0x58, 0x0a, 0x08,
	0x54, 0x79, 0x70, 0x65, 0x45, 0x6e, 0x75, 0x6d, 0x12, 0x0c, 0x0a, 0x08, 0x52, 0x70, 0x63, 0x54,
	0x69, 0x6d, 0x65, 0x72, 0x10, 0x00, 0x12, 0x13, 0x0a, 0x0f, 0x47, 0x65, 0x74, 0x4c, 0x6f, 0x67,
	0x69, 0x6e, 0x53, 0x65, 0x73, 0x73, 0x41, 0x72, 0x67, 0x10, 0x01, 0x12, 0x15, 0x0a, 0x11, 0x47,
	0x65, 0x74, 0x4c, 0x6f, 0x67, 0x69, 0x6e, 0x53, 0x65, 0x73, 0x73, 0x52, 0x65, 0x70, 0x6c, 0x79,
	0x10, 0x02, 0x12, 0x12, 0x0a, 0x0e, 0x47, 0x61, 0x6d, 0x65, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
	0x49, 0x6e, 0x66, 0x6f, 0x10, 0x03, 0x22, 0x29, 0x0a, 0x0f, 0x47, 0x65, 0x74, 0x4c, 0x6f, 0x67,
	0x69, 0x6e, 0x53, 0x65, 0x73, 0x73, 0x41, 0x72, 0x67, 0x12, 0x16, 0x0a, 0x06, 0x75, 0x73, 0x65,
	0x72, 0x49, 0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x03, 0x52, 0x06, 0x75, 0x73, 0x65, 0x72, 0x49,
	0x64, 0x22, 0x45, 0x0a, 0x11, 0x47, 0x65, 0x74, 0x4c, 0x6f, 0x67, 0x69, 0x6e, 0x53, 0x65, 0x73,
	0x73, 0x52, 0x65, 0x70, 0x6c, 0x79, 0x12, 0x16, 0x0a, 0x06, 0x75, 0x73, 0x65, 0x72, 0x49, 0x64,
	0x18, 0x01, 0x20, 0x01, 0x28, 0x03, 0x52, 0x06, 0x75, 0x73, 0x65, 0x72, 0x49, 0x64, 0x12, 0x18,
	0x0a, 0x07, 0x73, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52,
	0x07, 0x73, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x22, 0x80, 0x01, 0x0a, 0x0e, 0x47, 0x61, 0x6d,
	0x65, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x49, 0x6e, 0x66, 0x6f, 0x12, 0x1a, 0x0a, 0x08, 0x73,
	0x65, 0x72, 0x76, 0x65, 0x72, 0x49, 0x70, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08, 0x73,
	0x65, 0x72, 0x76, 0x65, 0x72, 0x49, 0x70, 0x12, 0x1e, 0x0a, 0x0a, 0x73, 0x65, 0x72, 0x76, 0x65,
	0x72, 0x4e, 0x61, 0x6d, 0x65, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0a, 0x73, 0x65, 0x72,
	0x76, 0x65, 0x72, 0x4e, 0x61, 0x6d, 0x65, 0x12, 0x18, 0x0a, 0x07, 0x6f, 0x72, 0x64, 0x65, 0x72,
	0x49, 0x64, 0x18, 0x03, 0x20, 0x01, 0x28, 0x05, 0x52, 0x07, 0x6f, 0x72, 0x64, 0x65, 0x72, 0x49,
	0x64, 0x12, 0x18, 0x0a, 0x07, 0x75, 0x73, 0x65, 0x72, 0x4e, 0x75, 0x6d, 0x18, 0x04, 0x20, 0x01,
	0x28, 0x05, 0x52, 0x07, 0x75, 0x73, 0x65, 0x72, 0x4e, 0x75, 0x6d, 0x42, 0x09, 0x5a, 0x07, 0x2e,
	0x3b, 0x72, 0x70, 0x63, 0x70, 0x62, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_rpc_proto_rawDescOnce sync.Once
	file_rpc_proto_rawDescData = file_rpc_proto_rawDesc
)

func file_rpc_proto_rawDescGZIP() []byte {
	file_rpc_proto_rawDescOnce.Do(func() {
		file_rpc_proto_rawDescData = protoimpl.X.CompressGZIP(file_rpc_proto_rawDescData)
	})
	return file_rpc_proto_rawDescData
}

var file_rpc_proto_enumTypes = make([]protoimpl.EnumInfo, 1)
var file_rpc_proto_msgTypes = make([]protoimpl.MessageInfo, 4)
var file_rpc_proto_goTypes = []interface{}{
	(PbType_TypeEnum)(0),      // 0: rpcpb.PbType.TypeEnum
	(*PbType)(nil),            // 1: rpcpb.PbType
	(*GetLoginSessArg)(nil),   // 2: rpcpb.GetLoginSessArg
	(*GetLoginSessReply)(nil), // 3: rpcpb.GetLoginSessReply
	(*GameServerInfo)(nil),    // 4: rpcpb.GameServerInfo
}
var file_rpc_proto_depIdxs = []int32{
	0, // [0:0] is the sub-list for method output_type
	0, // [0:0] is the sub-list for method input_type
	0, // [0:0] is the sub-list for extension type_name
	0, // [0:0] is the sub-list for extension extendee
	0, // [0:0] is the sub-list for field type_name
}

func init() { file_rpc_proto_init() }
func file_rpc_proto_init() {
	if File_rpc_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_rpc_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*PbType); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_rpc_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*GetLoginSessArg); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_rpc_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*GetLoginSessReply); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_rpc_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*GameServerInfo); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_rpc_proto_rawDesc,
			NumEnums:      1,
			NumMessages:   4,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_rpc_proto_goTypes,
		DependencyIndexes: file_rpc_proto_depIdxs,
		EnumInfos:         file_rpc_proto_enumTypes,
		MessageInfos:      file_rpc_proto_msgTypes,
	}.Build()
	File_rpc_proto = out.File
	file_rpc_proto_rawDesc = nil
	file_rpc_proto_goTypes = nil
	file_rpc_proto_depIdxs = nil
}
