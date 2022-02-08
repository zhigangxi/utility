package excel

type FieldType struct {
	InputFieldName string // 表中输入的类型
	GoFieldName    string //  转换为go的类型
	PBFieldName    string // 转换为pb的类型
	DefaultValue   string
}

// 将表中输入的字段类型转换为各种语言类型

var (
	fieldTypes = []*FieldType{
		{"int16", "int16", "int32", "0"},
		{"int32", "int32", "int32", "0"},
		{"int64", "int64", "int64", "0"},
		{"int", "int32", "int32", "0"},
		{"uint", "uint32", "uint32", "0"},
		{"uint16", "uint16", "uint32", "0"},
		{"uint32", "uint32", "uint32", "0"},
		{"uint64", "uint64", "uint64", "0"},
		{"float", "float32", "float", "0"},
		{"double", "float64", "double", "0"},
		{"float32", "float32", "float", "0"},
		{"float64", "float64", "double", "0"},
		{"bool", "bool", "bool", "false"},
		{"string", "string", "string", ""},
		{"int16[]", "[]int16", "repeated int32", "[]"},
		{"int32[]", "[]int32", "repeated int32", "[]"},
		{"int64[]", "[]int64", "repeated int64", "[]"},
		{"int[]", "[]int32", "repeated int32", "[]"},
		{"uint[]", "[]uint32", "repeated uint32", "[]"},
		{"uint16[]", "[]uint16", "repeated uint32", "[]"},
		{"uint32[]", "[]uint32", "repeated uint32", "[]"},
		{"uint64[]", "[]uint64", "repeated uint64", "[]"},
		{"float[]", "[]float32", "repeated float", "[]"},
		{"double[]", "[]float64", "repeated double", "[]"},
		{"float32[]", "[]float32", "repeated float", "[]"},
		{"float64[]", "[]float64", "repeated double", "[]"},
		{"bool[]", "[]bool", "repeated bool", "[]"},
		{"string[]", "[]string", "repeated string", "[]"},
	}

	fieldTypeByType = map[string]*FieldType{}
)

func init() {

	for _, ft := range fieldTypes {
		fieldTypeByType[ft.InputFieldName] = ft
	}
}
