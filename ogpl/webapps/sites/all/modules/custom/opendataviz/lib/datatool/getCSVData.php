<?php
class getCSVData {
  public function array_to_csv($array, $header_row = true, $col_sep = ",", $row_sep = "\n", $qut = '"')
  {
    if (!is_array($array) or !is_array($array[0])) return false;
    $output ='';
    //Header row.
    if ($header_row)
    {
      foreach ($array[0] as $key => $val)
      {
        //Escaping quotes.
        $key = str_replace($qut, "$qut$qut", $key);
        $output .= "$col_sep$qut$key$qut";
      }
      $output = substr($output, 1)."\n";
    }
    //Data rows.
    foreach ($array as $key => $val)
    {
      $tmp = '';
      foreach ($val as $cell_key => $cell_val)
      {
        //Escaping quotes.
        $cell_val = str_replace($qut, "$qut$qut", $cell_val);
        $tmp .= "$col_sep$qut$cell_val$qut";
      }
      $output .= substr($tmp, 1).$row_sep;
    }
     
    return $output;
  }
  
  public function getCSV($file, $worksheet = 0, $type = 'application/vnd.ms-excel', $format = 'csv') {
    require_once 'sites/all/modules/custom/dms_ds_upload/Classes/PHPExcel/IOFactory.php';
    $inputFileName = $file;
    if ($inputFileName != '') {
      if ( in_array($type, array('text/plain', 'application/vnd.ms-excel', 'text/csv','application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'))) {
        $objPHPExcel = PHPExcel_IOFactory::load($inputFileName);
        $objPHPExcel->setActiveSheetIndex($worksheet);
        $sheetData = $objPHPExcel->getActiveSheet()->toArray(null,true,true,true);
        $fields_array = array_slice($sheetData, 0, 1);
        $data_array = array_slice($sheetData, 1);
        if ($format == 'csv') {
          foreach($fields_array as $value){
            foreach($value as $field) {
              $fields[] = $field;
            }
          }
          foreach($data_array as $value){
            if(!empty($data_temp)) $datas[] = $data_temp;
            $data_temp =array();
            $i = 0;
            foreach($value as $data) {
              $key = $fields[$i];
              $data_temp[$key] = $data;
              $i++;
            }
          }
        }
      }
      else if ( in_array($type, array('text/xml', 'application/atom+xml','application/xml'))) {
        $xml = simplexml_load_file($inputFileName);
        foreach($xml as $value){
          $fields = array();
          foreach($value[0] as $key => $field) {
            $fields[] = $key;
          }
        }
        foreach($xml as $value){
          $datas[] = (array) $value;
        }
      }
      // Now process the file as per file type
      switch($format) {
        case 'csv':
          $csv =  self::array_to_csv($datas, TRUE);
          return $csv;
          break;
        case 'html':
          if ( !in_array($type, array('text/plain', 'application/vnd.ms-excel', 'text/csv', 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'))) {
            $objPHPExcel = new PHPExcel();
            $objPHPExcel->setActiveSheetIndex($worksheet);
            $activeSheet = $objPHPExcel->getActiveSheet();
            foreach ($fields as $i=>$col) {
              $activeSheet->setCellValue(self::getColLetter($i) . 1, $col);
            }
            $row = 1;
            foreach ($datas as $value) {
              $row++;
              $column = 0;
              foreach ($value as $data) {
                $activeSheet->setCellValue(self::getColLetter($column) . $row, $data);
                $column++;
              }
            }
          }
          else
          {
            $objPHPExcel->setActiveSheetIndex($worksheet);
          }
          break;
      }
    }
    else {
      return "No files";
    }
  }
}