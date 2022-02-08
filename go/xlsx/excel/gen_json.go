package excel

import (
	"bytes"
	"fmt"
	"strings"
)

func (c *CfgData) dataToJson(exportType string) *bytes.Buffer {
	var codeBuf bytes.Buffer
	codeBuf.WriteString("{\n")
	if !c.appendTableDataJson(&codeBuf, exportType) {
		return nil
	}
	codeBuf.WriteString("}\n")
	return &codeBuf
}

func (c *CfgData) appendTableDataJson(buf *bytes.Buffer, exprotType string) bool {
	firstTab := true
	for _, v := range c.ts {
		if len(v.exportType) > 0 && v.exportType != exprotType {
			continue
		}
		if !firstTab {
			fmt.Fprintf(buf, ",\n")
		}
		typeName := strings.Title(v.name)
		if v.attr.isKv {
			fmt.Fprintf(buf, "\"%sData\": {\n", typeName)
		} else {
			fmt.Fprintf(buf, "\"%sData\": [\n", typeName)
		}
		dataNum := len(v.fs[0].val)
		for i := 0; i < dataNum; i++ {
			if !v.attr.isKv {
				fmt.Fprintf(buf, "{\n")
			}
			firstVal := true
			for _, f := range v.fs {
				if len(f.exportType) == 0 || f.exportType == exprotType {
					if !firstVal {
						fmt.Fprintf(buf, ",\n")
					}
					if !c.appendFieldVal(buf, f.valType, f.valName, f.val[i], v.attr.split) {
						fmt.Printf("%s:%s\n", v.xlsxName, v.sheetName)
						return false
					}
					firstVal = false
				}
			}
			if !v.attr.isKv {
				if dataNum > i+1 {
					fmt.Fprintf(buf, "\n},\n")
				} else {
					fmt.Fprintf(buf, "\n}\n")
				}
			}
		}
		if v.attr.isKv {
			fmt.Fprint(buf, "}\n")
		} else {
			fmt.Fprint(buf, "]\n")
		}
		firstTab = false
	}
	return true
}

func (c *CfgData) appendFieldVal(buf *bytes.Buffer, valType, valName, val, split string) bool {
	if len(val) == 0 {
		if isString(valType) && !isArray(valType) {
			fmt.Fprintf(buf, "\t\"%s\":\"\"", valName)
		} else if c.isEnumType(valType) {
			fmt.Printf("枚举类型:%s 不能配置为空", valType)
			return false
		} else {
			fmt.Fprintf(buf, "\t\"%s\":%s", valName, fieldTypeByType[valType].DefaultValue)
		}
		return true
	}
	valIsString := isString(valType)
	valIsEnum := c.isEnumType(valType)
	if isArray(valType) {
		var allVal string
		valArray := strings.Split(val, split)
		if valIsString {
			for _, v := range valArray {
				if len(allVal) > 0 {
					allVal = fmt.Sprintf("%s,\"%s\"", allVal, v)
				} else {
					allVal = fmt.Sprintf("\"%s\"", v)
				}
			}
		} else if valIsEnum {
			for _, v := range valArray {
				if c.enumValType[v] != valType {
					fmt.Printf("枚举类型:%s 没有:%s\n", valType, v)
					return false
				}
				intVal := c.allEnumVal[v].val
				if len(allVal) > 0 {
					allVal = fmt.Sprintf("%s,%d", allVal, intVal)
				} else {
					allVal = fmt.Sprintf("%d", intVal)
				}
			}
		} else {
			for _, v := range valArray {
				if len(allVal) > 0 {
					allVal = fmt.Sprintf("%s,%s", allVal, v)
				} else {
					allVal = v
				}
			}
		}
		fmt.Fprintf(buf, "\t\"%s\":[%s]", valName, allVal)
	} else {
		if valIsString {
			fmt.Fprintf(buf, "\t\"%s\":\"%s\"", valName, val)
		} else if valIsEnum {
			if c.enumValType[val] != valType {
				fmt.Printf("枚举类型:%s 没有:%s\n", valType, val)
				return false
			}
			intVal := c.allEnumVal[val].val
			fmt.Fprintf(buf, "\t\"%s\":%d", valName, intVal)
		} else {
			fmt.Fprintf(buf, "\t\"%s\":%s", valName, val)
		}
	}
	return true
}
