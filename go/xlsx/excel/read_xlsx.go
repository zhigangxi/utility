package excel

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"strings"

	"go/format"

	"github.com/golang/protobuf/jsonpb"
	"github.com/golang/protobuf/proto"
	"github.com/jhump/protoreflect/desc/protoparse"
	"github.com/jhump/protoreflect/dynamic"
	"github.com/xuri/excelize/v2"
)

//"fmt"

//"github.com/xuri/excelize/v2"

const (
	SERVER          = "server"
	CLIENT          = "client"
	TABLE_NAME      = "表"
	ENUM_NAME       = "枚举"
	DATA_TYPE_SHEET = "数据类型"
	PACKAGE_NAME    = "config"
)

type CfgData struct {
	ts          []*table
	enumMap     map[string]*tabEnum
	allEnumName map[string]string
	allEnumVal  map[string]enumValFilePath
	enumValType map[string]string //valName,typeName
}

type enumValFilePath struct {
	val      int32
	filePath string
}

type table struct {
	attr       attributes // 属性
	fs         []field    // 字段
	name       string
	exportType string
	xlsxName   string
	sheetName  string
}

type attributes struct {
	key        string // 主键字段名
	split      string
	isKv       bool // 是否是key-val表
	haveExpand bool // 是否有扩展字段
}

type tabEnum struct {
	typeName string
	valName  []string
	val      []int32
}

type field struct {
	valName    string
	valType    string
	exportType string //server|client
	val        []string
}

func NewCfgData() *CfgData {
	return new(CfgData)
}

func getAllXlsxFile(path string) []string {
	var files []string
	infos, err := ioutil.ReadDir(path)
	if err != nil {
		fmt.Printf("Read dir error:%s(%s)\n", path, err.Error())
		return nil
	}
	for _, v := range infos {
		if !v.IsDir() && strings.Contains(v.Name(), ".xlsx") && !strings.Contains(v.Name(), "~$") {
			files = append(files, v.Name())
		}
	}
	return files
}

func (c *CfgData) ReadData(xlsxPath, xlsxFile string) bool {
	var files []string
	if len(xlsxFile) > 0 {
		files = strings.Split(xlsxFile, " ")
	} else {
		files = getAllXlsxFile(xlsxPath)
	}
	for _, v := range files {
		if !c.readFileData(xlsxPath + "/" + v) {
			return false
		}
	}
	return true
}

func (c *CfgData) readFileData(file string) bool {
	f, err := excelize.OpenFile(file)
	if err != nil {
		fmt.Println(err)
		return false
	}
	defer func() {
		if err := f.Close(); err != nil {
			fmt.Println(err)
		}
	}()
	tabDataMap, ok := c.readTypeData(f)
	if !ok {
		//fmt.Printf("没有 \"%s\" 页签", DATA_TYPE_SHEET)
		fmt.Printf("读取数据类型失败:%s\n", file)
		return false
	}
	allTable, ok := c.readSheetData(f, tabDataMap)
	if !ok {
		fmt.Printf("读取数据失败:%s\n", file)
		return false
	}
	c.ts = allTable
	return true
}

func (c *CfgData) readSheetData(file *excelize.File, sheets map[string]tableTypeData) ([]*table, bool) {
	var data []*table
	for k, v := range sheets {
		t := c.readTableData(file, k, v.dataName, v.exportType)
		if t == nil {
			return nil, false
		}
		data = append(data, t)
	}
	return data, true
}

func ParseBool(s string) (bool, error) {
	switch s {
	case "是", "yes", "1", "true":
		return true, nil
	case "否", "no", "0", "false":
		return false, nil
	case "":
		return false, nil
	}
	return false, errors.New("invalid bool value")
}

func (c *CfgData) readTableData(file *excelize.File, sheetName, dataTypeName, exportType string) *table {
	var data table
	data.name = dataTypeName
	data.exportType = exportType
	data.xlsxName = file.Path
	data.sheetName = sheetName
	rows, err := file.GetRows(sheetName)
	if err != nil {
		fmt.Printf("%s error:%s", sheetName, err.Error())
		return nil
	}
	if len(rows) < 3 {
		fmt.Printf("%s 行数少于三行，格式错误\n", sheetName)
		return nil
	}
	lineAttr := rows[1]
	if len(lineAttr) < 4 {
		fmt.Printf("%s 属性行少于4列，格式错误\n", sheetName)
		return nil
	}

	//索引	数组切割	是否KV表	是否有扩展字段
	data.attr.key = strings.Title(lineAttr[0])
	data.attr.split = lineAttr[1]
	data.attr.isKv, err = ParseBool(strings.ToLower(lineAttr[2]))
	if err != nil {
		fmt.Printf("%s 错误的布尔类型:%s\n", sheetName, lineAttr[2])
		return nil
	}
	data.attr.haveExpand, err = ParseBool(strings.ToLower(lineAttr[3]))
	if err != nil {
		fmt.Printf("%s 错误的布尔类型:%s\n", sheetName, lineAttr[3])
		return nil
	}

	if data.attr.isKv {
		if len(rows) < 5 {
			fmt.Printf("%s 没有数据\n", sheetName)
			return nil
		}
		fields := c.getKVTableField(rows[4:])
		if fields == nil {
			fmt.Printf("%s 解析字段出错\n", sheetName)
			return nil
		}
		data.fs = fields
	} else {
		if len(rows) < 8 {
			fmt.Printf("%s 没有数据\n", sheetName)
			return nil
		}
		fields := c.getDataTableField(rows[4:])
		if fields == nil {
			fmt.Printf("%s 解析字段出错\n", sheetName)
			return nil
		}
		if len(data.attr.key) > 0 {
			findKey := false
			for _, v := range fields {
				if data.attr.key == v.valName {
					if isArray(v.valType) {
						fmt.Printf("%s 索引字段类型不能是数组\n", sheetName)
						return nil
					}
					if haveRepeatVal(v.val) {
						fmt.Printf("%s 索引字段数据不能重复\n", sheetName)
						return nil
					}
					findKey = true
					break
				}
			}
			if !findKey {
				fmt.Printf("%s 索引字段未找到\n", sheetName)
				return nil
			}
		}
		data.fs = fields
	}
	return &data
}

func haveRepeatVal(vals []string) bool {
	strMap := make(map[string]bool)
	for _, v := range vals {
		if strMap[v] {
			return true
		}
		strMap[v] = true
	}
	return false
}

func getRowData(row []string, pos int) string {
	if pos >= 0 && pos < len(row) {
		return row[pos]
	}
	return ""
}

func getFileNum(datas []string) int {
	num := len(datas)
	for k, v := range datas {
		if len(v) <= 0 {
			num = k
			break
		}
	}
	return num
}

func (c *CfgData) getDataTableField(datas [][]string) []field {
	fieldNum := getFileNum(datas[0])
	if fieldNum != getFileNum(datas[1]) {
		return nil
	}
	for k, v := range datas[1] {
		if fieldTypeByType[v] == nil && !c.isEnumType(v) {
			fmt.Printf("错误的数据类型:%s\n", v)
			return nil
		}
		exportType := getRowData(datas[2], k)
		if exportType == "" {
			continue
		}
		if exportType != SERVER && exportType != CLIENT {
			fmt.Printf("导出类型错误:%s\n", exportType)
			return nil
		}
	}
	dataNum := len(datas) - 3
	if dataNum <= 0 {
		return nil
	}
	var fs []field
	for i := 0; i < fieldNum; i++ {
		var f field
		f.valName = strings.Title(datas[0][i])
		f.valType = datas[1][i]
		f.exportType = getRowData(datas[2], i)
		fs = append(fs, f)
	}
	fielsData := datas[3:]
	for i := 0; i < dataNum; i++ {
		if fielsData[i][0][0] == '#' {
			//注释行
			continue
		}
		for j := 0; j < fieldNum; j++ {
			f := getRowData(fielsData[i], j)
			fs[j].val = append(fs[j].val, f)
			if c.isEnumType(fs[j].valType) && len(fs[j].val) == 0 {
				fmt.Printf("枚举类型:%s 不能配置为空", fs[j].valName)
				return nil
			}
		}
	}
	return fs
}

func (c *CfgData) isEnumType(name string) bool {
	trimName := strings.Trim(name, "[]")
	_, ok := c.allEnumName[trimName]
	return ok
}

func (c *CfgData) getKVTableField(datas [][]string) []field {
	var fs []field
	for _, v := range datas {
		if v[0][0] == '#' {
			//注释行
			continue
		}
		if len(v) < 3 {
			fmt.Printf("%v 配置错误", v)
			return nil
		}
		if fieldTypeByType[v[1]] == nil && !c.isEnumType(v[1]) {
			fmt.Printf("错误的数据类型:%s\n", v)
			return nil
		}
		exportType := getRowData(v, 3)
		if len(exportType) > 0 && exportType != SERVER && exportType != CLIENT {
			fmt.Printf("导出类型错误:%s\n", exportType)
			return nil
		}
		var f field
		f.valName = strings.Title(v[0])
		f.valType = v[1]
		f.val = append(f.val, v[2])
		f.exportType = exportType
		fs = append(fs, f)
	}
	return fs
}

// type typeDataVal struct {
// 	enumMap map[string]*tabEnum
// 	dataMap map[string]string // 页签名,数据类型名
// }

func (c *CfgData) addEnumData(enumType, enumName, filepath string) bool {
	if c.enumMap == nil {
		c.enumMap = make(map[string]*tabEnum)
	}
	if c.allEnumName == nil {
		c.allEnumName = make(map[string]string)
	}
	if c.allEnumVal == nil {
		c.allEnumVal = make(map[string]enumValFilePath)
	}
	if c.enumValType == nil {
		c.enumValType = make(map[string]string)
	}
	e := c.enumMap[enumType] //(*enumMap)[enumType]
	var val int32
	if e == nil {
		e = new(tabEnum)
		e.typeName = enumType
		e.valName = append(e.valName, enumName)
		val = int32(len(e.val)) + 1
		e.val = append(e.val, val)
		c.enumMap[enumType] = e
	} else {
		val = int32(len(e.val)) + 1
		e.valName = append(e.valName, enumName)
		e.val = append(e.val, val)
	}
	if typeFile, ok := c.allEnumName[enumType]; ok {
		if typeFile != filepath {
			fmt.Printf("枚举:%s 已经在文件:%s 定义过\n", enumName, typeFile)
			return false
		}
	}
	if valFile, ok := c.allEnumVal[enumName]; ok {
		fmt.Printf("枚举类型:%s 已经在文件:%s 定义过\n", enumName, valFile.filePath)
		return false
	}
	c.allEnumName[enumType] = filepath
	var valFile enumValFilePath
	valFile.val = val
	valFile.filePath = filepath
	c.allEnumVal[enumName] = valFile
	c.enumValType[enumName] = enumType
	return true
}

type tableTypeData struct {
	dataName   string
	exportType string
}

func (c *CfgData) readTypeData(file *excelize.File) (map[string]tableTypeData, bool) {
	//DATA_TYPE_SHEET
	dataMap := make(map[string]tableTypeData)

	rows, err := file.GetRows(DATA_TYPE_SHEET)
	if err != nil {
		fmt.Printf("%s error:%s", DATA_TYPE_SHEET, err.Error())
		return nil, false
	}
	for k, v := range rows {
		if k == 0 {
			continue
		}
		if len(v) < 3 {
			fmt.Printf("读取 %s 第%d行出错\n", DATA_TYPE_SHEET, k+1)
			return nil, false
		}
		exportType := getRowData(v, 3)
		if v[0] == TABLE_NAME {
			if len(exportType) > 0 && exportType != SERVER && exportType != CLIENT {
				fmt.Printf("导出类型错误:%s\n", exportType)
				return nil, false
			}
			var typeData tableTypeData
			typeData.dataName = v[2]
			typeData.exportType = exportType
			dataMap[v[1]] = typeData
		} else if v[0] == ENUM_NAME {
			if isString(v[1]) {
				fmt.Printf("%s 第%d行:\"%s\" 枚举类型不能包含string\n", DATA_TYPE_SHEET, k+1, v[0])
				return nil, false
			}
			if !c.addEnumData(v[1], v[2], file.Path) {
				fmt.Printf("%s 第%d行:\"%s\"\n", DATA_TYPE_SHEET, k+1, v[0])
				return nil, false
			}
		} else {
			fmt.Printf("%s 第%d行:\"%s\"未知类型\n", DATA_TYPE_SHEET, k+1, v[0])
			return nil, false
		}
	}
	return dataMap, true
}

func (c *CfgData) ExportServerData(serverPath string) {
	os.MkdirAll(serverPath, 0755)
	var dataFile = serverPath + "/" + "config.go"

	buf := c.toGoBuf()
	source, err := format.Source(buf.Bytes())
	if err != nil {
		ioutil.WriteFile(dataFile, buf.Bytes(), 0666)
		fmt.Printf("源代码有错: %s \n %s", dataFile, err.Error())
		return
	}
	ioutil.WriteFile(dataFile, source, 0666)
	c.ExportJsonData(serverPath, SERVER)
}

func (c *CfgData) ExportJsonData(path, exprotType string) {
	os.MkdirAll(path, 0755)
	var dataFile = path + "/" + "data.json"

	buf := c.dataToJson(exprotType)
	if buf == nil {
		return
	}

	var formatJson bytes.Buffer
	err := json.Indent(&formatJson, buf.Bytes(), "", "\t")
	if err != nil {
		ioutil.WriteFile(dataFile, buf.Bytes(), 0666)
		fmt.Printf("源代码有错: %s \n %s", dataFile, err.Error())
		return
	}
	ioutil.WriteFile(dataFile, formatJson.Bytes(), 0666)
}

func (c *CfgData) ExportPbData(path, exprotType string) {
	os.MkdirAll(path, 0755)
	var jsonDataFile = path + "/" + "data.json"
	var pbDataFile = path + "/" + "data.pb"
	var protoFile = path + "/" + "data.proto"

	buf := c.dataToJson(exprotType)
	if buf == nil {
		return
	}

	var formatJson bytes.Buffer
	err := json.Indent(&formatJson, buf.Bytes(), "", "\t")
	if err != nil {
		ioutil.WriteFile(jsonDataFile, buf.Bytes(), 0666)
		fmt.Printf("源代码有错: %s \n %s", jsonDataFile, err.Error())
		return
	}
	ioutil.WriteFile(jsonDataFile, formatJson.Bytes(), 0666)

	buf = c.toProto(exprotType)
	ioutil.WriteFile(protoFile, buf.Bytes(), 0666)
	var parser protoparse.Parser
	fileDesc, protoErr := parser.ParseFiles(protoFile)
	if protoErr != nil {
		fmt.Printf("proto 文件有错误: %s \n %s", protoFile, protoErr.Error())
		return
	}
	msgDesc := fileDesc[0].FindMessage(fmt.Sprintf("%s.ConfigData", PACKAGE_NAME))
	pb := dynamic.NewMessage(msgDesc)
	err = jsonpb.UnmarshalString(formatJson.String(), pb)
	//fmt.Println(pb)
	if err != nil {
		fmt.Printf("json 序列换pb出错:%s", err.Error())
		return
	}
	data, pbErr := proto.Marshal(pb)
	if pbErr != nil {
		fmt.Printf("proto 文件有错误: %s \n %s", pbDataFile, pbErr.Error())
		return
	}
	ioutil.WriteFile(pbDataFile, data, 0666)
}
