package excel

import (
	"bytes"
	"fmt"
	"strings"
)

func (c *CfgData) toGoBuf() *bytes.Buffer {
	headStr := fmt.Sprintf("// Code generated by config tool. DO NOT EDIT.\npackage %s\n\nimport (\n\"encoding/json\"\n\"fmt\"\n\"io/ioutil\"\n)\n", PACKAGE_NAME)
	var codeBuf bytes.Buffer
	codeBuf.WriteString(headStr)
	c.appendEnumToGoStr(&codeBuf)
	codeBuf.WriteString("\n")
	c.appendTableGoStr(&codeBuf)
	codeBuf.WriteString("\n")
	c.appendTableStructGoStr(&codeBuf)
	codeBuf.WriteString("\n")
	c.appendFuncGoStr(&codeBuf)
	codeBuf.WriteString("\n")
	return &codeBuf
}

func isArray(str string) bool {
	return strings.Contains(str, "[]")
}

func isString(str string) bool {
	return strings.Contains(str, "string")
}

func (c *CfgData) appendTableGoStr(buf *bytes.Buffer) {
	for _, v := range c.ts {
		if v.exportType == CLIENT {
			continue
		}
		typeName := strings.Title(v.name)
		fmt.Fprintf(buf, "type %s struct {\n", typeName)
		for _, f := range v.fs {
			if f.exportType != CLIENT {
				valName := f.valName
				var valType string
				fieldType := fieldTypeByType[f.valType]
				if fieldType != nil {
					valType = fieldType.GoFieldName
				} else {
					valType = f.valType
				}
				fmt.Fprintf(buf, "\t%s %s\n", valName, valType)
			}
		}
		if v.attr.haveExpand {
			fmt.Fprintf(buf, "\tExt interface{}\n")
		}
		fmt.Fprint(buf, "}\n\n")
	}
}

//gofmt -l -w -s
func (c *CfgData) appendEnumToGoStr(buf *bytes.Buffer) {
	//type RechargeType int32
	for k, v := range c.enumMap {
		enumName := k
		fmt.Fprintf(buf, "type %s int32\nconst (\n", enumName)
		for pos, name := range v.valName {
			fmt.Fprintf(buf, "\t%s %s = %d\n", name, enumName, v.val[pos])
		}
		fmt.Fprint(buf, ")\n")
	}
}

func (c *CfgData) appendTableStructGoStr(buf *bytes.Buffer) {
	fmt.Fprintf(buf, "type ConfigData struct {\n")
	for _, v := range c.ts {
		if v.exportType == CLIENT {
			continue
		}
		typeName := strings.Title(v.name)
		if v.attr.isKv {
			fmt.Fprintf(buf, "\t%sData *%s\n", typeName, typeName)
		} else {
			fmt.Fprintf(buf, "\t%sData []*%s\n", typeName, typeName)
			if len(v.attr.key) > 0 {
				var keyType string
				for _, f := range v.fs {
					if v.attr.key == f.valName {
						keyType = f.valType
					}
				}
				if !c.isEnumType(keyType) {
					keyType = fieldTypeByType[keyType].GoFieldName
				}
				fmt.Fprintf(buf, "\t%sDataBy%s map[%s]*%s `json:\"-\"`\n", typeName, v.attr.key, keyType, typeName)
			}
		}
	}
	fmt.Fprintf(buf, "\tpostHandlers []func(*ConfigData) `json:\"-\"`\n")
	fmt.Fprintf(buf, "}\n")
}

func (c *CfgData) appendFuncGoStr(buf *bytes.Buffer) {
	funcCode := `var (
		cfgData ConfigData
	)
	
	func LoadConfigData(file string)bool{
		data, err := ioutil.ReadFile(file)
		if err != nil{
			fmt.Println(err.Error())
			return false
		}
		err = json.Unmarshal(data, &cfgData)
		if err != nil {
			fmt.Println(err.Error())
			return false
		}
		%s
		for _,v:=range cfgData.postHandlers{
			v(&cfgData)
		}
		return true
	}

	func RegisterLoadHandler(f func(*ConfigData)){
		cfgData.postHandlers = append(cfgData.postHandlers, f)
	}`

	var mapBuf, getDataFuncBuf bytes.Buffer
	for _, v := range c.ts {
		if v.exportType == CLIENT {
			continue
		}
		typeName := strings.Title(v.name)
		if !v.attr.isKv {
			if len(v.attr.key) > 0 {
				var keyType string
				for _, f := range v.fs {
					if v.attr.key == f.valName {
						keyType = f.valType
						break
					}
				}
				if !c.isEnumType(keyType) {
					keyType = fieldTypeByType[keyType].GoFieldName
				}
				mapName := fmt.Sprintf("cfgData.%sDataBy%s", typeName, v.attr.key)
				fmt.Fprintf(&mapBuf, "\t%s = make(map[%s]*%s)\n", mapName, keyType, typeName)
				fmt.Fprintf(&mapBuf, "\tfor _, v := range cfgData.%sData {\n", typeName)
				fmt.Fprintf(&mapBuf, "\t if %s[v.%s] != nil{\n return false\n}\n", mapName, v.attr.key)
				fmt.Fprintf(&mapBuf, "\t%s[v.%s] = v\n", mapName, v.attr.key)
				fmt.Fprintf(&mapBuf, "\t}\n")

				fmt.Fprintf(&getDataFuncBuf, "\nfunc Get%sBy%s(key %s)*%s{\n return %s[key]\n}\n", typeName, v.attr.key, keyType, typeName, mapName)
			}
			fmt.Fprintf(&getDataFuncBuf, "\nfunc Get%s()[]*%s{\n return cfgData.%sData\n}\n", typeName, typeName, typeName)
		} else {
			fmt.Fprintf(&getDataFuncBuf, "\nfunc Get%s()*%s{\n return cfgData.%sData\n}\n", typeName, typeName, typeName)
		}
	}

	fmt.Fprintf(buf, funcCode, mapBuf.String())
	fmt.Fprintf(buf, "\n%s\n", getDataFuncBuf.String())
}
