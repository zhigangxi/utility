package main

import (
	"flag"

	"xlsx/excel"
)

var (
	xlsxPath   = flag.String("xlsx_path", "./file", "xlsx file path")
	xlsxFile   = flag.String("xlsx_file", "", "xlsx files")
	serverPath = flag.String("server", "./server", "server file path")
	clientPath = flag.String("client", "./client", "client file path")
	//clientDataType = flag.String("cd_type", "json", "export client data type(json|pb)")
)

func main() {
	flag.Parse()
	config := excel.NewCfgData()
	//fmt.Printf("xlsx path:%s\n", *xlsxPath)
	if config.ReadData(*xlsxPath, *xlsxFile) {
		config.ExportServerData(*serverPath)
		//config.ExportJsonData(*clientPath, excel.CLIENT)
		config.ExportPbData(*clientPath, excel.CLIENT)
	}
	// type ConfigData struct {
	// 	ATableDataData []*serverCfg.ATableData
	// 	KvDataData     *serverCfg.KvData
	// }
	// data, err := ioutil.ReadFile("server/data.json")
	// var conf ConfigData
	// if err == nil {
	// 	err = json.Unmarshal(data, &conf)
	// 	if err != nil {
	// 		fmt.Println(err.Error())
	// 	}
	// } else {
	// 	fmt.Println(err.Error())
	// }
	// fmt.Printf("%s,%s,%s,%s", *xlsxFile, *xlsxPath, *serverPath, *clientPath)
	// f, err := excelize.OpenFile("book.xlsx")
	// if err != nil {
	// 	fmt.Println(err)
	// 	return
	// }
	// defer func() {
	// 	if err := f.Close(); err != nil {
	// 		fmt.Println(err)
	// 	}
	// }()
	// name := f.GetSheetName(0)
	// rows, _ := f.GetRows(name)
	// fmt.Println(rows)
}
