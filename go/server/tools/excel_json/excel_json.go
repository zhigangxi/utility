package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strings"
	"syscall"

	"github.com/tealeg/xlsx"
)

type xlsData struct {
	tableName string           //表名
	whoUse    []string         //客户端/服务器使用
	typeName  []string         //类型
	varName   []string         //变量名
	varData   map[int][]string //变量内容
}

func initXlsData(data *xlsData, cellNum int) {
	data.typeName = make([]string, cellNum)
	data.varName = make([]string, cellNum)
	data.whoUse = make([]string, cellNum)
	data.varData = make(map[int][]string)
}

func getSheetData(sheet *xlsx.Sheet, data *xlsData) {
	data.tableName = sheet.Name
	if len(sheet.Rows) < 4 {
		fmt.Printf("文件格式错误:%s\n", sheet.Name)
		return
	}
	//fmt.Println(data.tableName)
	// if data.tableName == "fish_group" {
	// 	fmt.Printf("fish_group")
	// }
	cellNum := len(sheet.Rows[0].Cells)
	initXlsData(data, cellNum)
	//fmt.Print(len(sheet.Rows))
	for i, row := range sheet.Rows {
		if i == 0 {
			continue
		}
		for j, cell := range row.Cells {
			if j >= cellNum {
				break
			}
			if i == 1 {
				data.varData[j] = make([]string, len(sheet.Rows)-4)
			}
			if i == 1 {
				data.whoUse[j] = cell.String()
			} else if i == 2 {
				data.typeName[j] = cell.String()
			} else if i == 3 {
				data.varName[j] = cell.String()
			} else {
				if j < len(data.varData) {
					data.varData[j][i-4] = cell.String()
				}
			}
		}
	}
}

func getXlsData(fileName string) []xlsData {
	xlFile, err := xlsx.OpenFile(fileName)
	if err != nil {
		fmt.Printf("can not open file:%s\n", fileName)
		return nil
	}
	var num = len(xlFile.Sheets)
	var datas = make([]xlsData, num)
	for s, sheet := range xlFile.Sheets {
		getSheetData(sheet, &datas[s])
	}
	// if strings.Contains(fileName, "star5.xlsx") {
	// 	fmt.Println(fileName)
	// 	fmt.Println(datas)
	// }
	return datas
}

func writeToJson(data xlsData, name string) {
	var num = len(data.varData[0])
	// if data.tableName == "fish_type" {
	// 	fmt.Printf("fish_group")
	// }
	serverStr := "[\n"
	clientStr := "[\n"
	for i := 0; i < num; i++ {
		serverStr += "{"
		clientStr += "{"
		var addServer = false
		var addClient = false
		for j := 0; j < len(data.varName); j++ {
			if data.whoUse[j] == "共用" {
				if addClient {
					clientStr += ","
				}
				if addServer {
					serverStr += ","
				}
				if data.typeName[j] == "int" {
					serverStr += fmt.Sprintf(`"%s":%s`, data.varName[j], data.varData[j][i])
					clientStr += fmt.Sprintf(`"%s":%s`, data.varName[j], data.varData[j][i])
				} else {
					serverStr += fmt.Sprintf(`"%s":"%s"`, data.varName[j], data.varData[j][i])
					clientStr += fmt.Sprintf(`"%s":"%s"`, data.varName[j], data.varData[j][i])
				}
				addServer = true
				addClient = true
			} else if data.whoUse[j] == "客户端" {
				if addClient {
					clientStr += ","
				}
				if data.typeName[j] == "int" {
					clientStr += fmt.Sprintf(`"%s":%s`, data.varName[j], data.varData[j][i])
				} else {
					clientStr += fmt.Sprintf(`"%s":"%s"`, data.varName[j], data.varData[j][i])
				}
				addClient = true
			} else if data.whoUse[j] == "服务器" {
				if addServer {
					serverStr += ","
				}
				if data.typeName[j] == "int" {
					serverStr += fmt.Sprintf(`"%s":%s`, data.varName[j], data.varData[j][i])
				} else {
					serverStr += fmt.Sprintf(`"%s":"%s"`, data.varName[j], data.varData[j][i])
				}
				addServer = true
			}
			// if data.tableName == "fish_group" {
			// 	fmt.Println(serverStr)
			// }
		}
		serverStr += "}"
		clientStr += "}"
		if i+1 < num && len(data.varData[0][i+1]) <= 0 {
			break
		}
		if i != num-1 {
			serverStr += ",\n"
			clientStr += ",\n"
		}
	}
	serverStr += "\n]"
	clientStr += "\n]"
	var serverFile = fmt.Sprintf("server/%s_%s.json", name, data.tableName)
	err := os.Remove(serverFile)
	if err != nil {
		//fmt.Println(err)
	}
	err = ioutil.WriteFile(serverFile, []byte(serverStr), syscall.O_CREAT)
	if err != nil {
		fmt.Println(err)
	}
	var clientFile = fmt.Sprintf("client/%s_%s.json", name, data.tableName)
	err = os.Remove(clientFile)
	if err != nil {
		//fmt.Println(err)
	}
	err = ioutil.WriteFile(clientFile, []byte(clientStr), syscall.O_CREAT)
	if err != nil {
		fmt.Println(err)
	}
}

func fullPath(path string) string {
	absolutePath, _ := filepath.Abs(path)
	return absolutePath
}

func getFullPath(path string) string {
	absolutePath, _ := filepath.Abs(path)
	return absolutePath
}

func eatchFile(path string) {
	if strings.Contains(path, ".xlsx") {
		createJsonFile(path)
		return
	}
	fullPath := getFullPath(path)

	filepath.Walk(fullPath, func(path string, fi os.FileInfo, err error) error {
		if nil == fi {
			return err
		}
		name := fullPath + "/" + fi.Name()
		if fi.IsDir() {
			//eatchFile(name)
			return nil
		}

		// if nameLen > 4 {
		// 	fileType := name[nameLen-5:]
		// 	if fileType == ".xlsx" {
		// 		createJsonFile(path)
		// 	}
		// }
		if strings.Contains(name, ".xlsx") {
			//if strings.Contains(name, "star5.xlsx") {
			createJsonFile(path)
			//}
		}
		return nil
	})
}

func createJsonFile(name string) {
	var datas = getXlsData(name)
	if datas == nil {
		return
	}
	_, file := path.Split(name)
	last := strings.LastIndex(file, "\\")
	if last < 0 {
		last = strings.LastIndex(file, "/")
	}
	file = file[last+1 : len(file)-5]
	for _, data := range datas {
		writeToJson(data, file)
	}
}

func main() {
	fmt.Println("请输入文件或者路径")
	reader := bufio.NewReader(os.Stdin)
	data, _, _ := reader.ReadLine()
	path := string(data)
	//var path = "E:\\svn\\master\\doc\\数据表"
	os.MkdirAll("server", os.ModeDir)
	os.MkdirAll("client", os.ModeDir)
	eatchFile(path)
	//eatchFile("E:\\temp\\data")
	fmt.Println("转换完成")
	reader.ReadLine()
}
