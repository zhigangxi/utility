package excel

import (
	"bytes"
	"fmt"
	"strings"
)

func (c *CfgData) toProto(exportType string) *bytes.Buffer {
	headStr := fmt.Sprintf("// Code generated by config tool. DO NOT EDIT.\nsyntax = \"proto3\";\npackage %s;\n\n", PACKAGE_NAME)
	var codeBuf bytes.Buffer
	codeBuf.WriteString(headStr)
	c.appendEnumToProtoStr(&codeBuf)
	codeBuf.WriteString("\n\n")
	c.appendTableProtoStr(&codeBuf, exportType)
	return &codeBuf
}

func (c *CfgData) appendTableProtoStr(buf *bytes.Buffer, exprotType string) {
	for _, v := range c.ts {
		if len(v.exportType) > 0 && v.exportType != exprotType {
			continue
		}
		typeName := strings.Title(v.name)
		fmt.Fprintf(buf, "message %s {\n", typeName)
		id := 1
		for _, f := range v.fs {
			if len(f.exportType) == 0 || f.exportType == exprotType {
				valName := f.valName
				var valType string
				fieldType := fieldTypeByType[f.valType]
				if fieldType != nil {
					valType = fieldType.PBFieldName
				} else {
					valType = f.valType
				}
				fmt.Fprintf(buf, "\t%s %s = %d;\n", valType, valName, id)
				id++
			}
		}
		fmt.Fprint(buf, "}\n\n")
	}

	fmt.Fprintf(buf, "message %s {\n", "ConfigData")
	for k, v := range c.ts {
		typeName := strings.Title(v.name)
		if v.attr.isKv {
			fmt.Fprintf(buf, "\t%s %sData = %d;\n", typeName, typeName, k+1)
		} else {
			fmt.Fprintf(buf, "\trepeated %s %sData = %d;\n", typeName, typeName, k+1)
		}
	}
	fmt.Fprint(buf, "}\n\n")
}

func (c *CfgData) appendEnumToProtoStr(buf *bytes.Buffer) {
	for k, v := range c.enumMap {
		enumName := k
		fmt.Fprintf(buf, "enum %s { \n", enumName)
		fmt.Fprintf(buf, "\t%sNone = 0;\n", enumName)
		for pos, name := range v.valName {
			fmt.Fprintf(buf, "\t%s = %d;\n", name, v.val[pos])
		}
		fmt.Fprint(buf, "}\n\n")
	}
}