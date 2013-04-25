var csv_arr = {};
var processed_data = {};
var prev_strep = '';
function odv_clean_input(input) {
	
	input = (typeof wms_replace_script != 'undefined' ? wms_replace_script(input) : input);
	
	return input;
}

$(function() {
	$("#proces-csv").click(function() {
		if ($('#csv-data').val() != '') {
			var csv = $('#csv-data').val();
			
			csv = odv_clean_input(csv);
			
			csv_arr = $.csv.toArrays(csv);
			
			reGeneratedata(csv_arr);
		}
		prev_strep = 'proces-csv';
	});
	
	$("#preview-csv").click(function() {
		if(prev_strep == 'proces-csv') {
			processed_data = processHeader(csv_arr);
			
			if(processed_data == null) {
				return null;
			}
			
			$('#preview-data').html('');
			
			var new_data = generateTable(processed_data);
			
			new_data = odv_clean_input(new_data);
			
			$('#preview-data').html(new_data);
			
			var csv_data = generatCSV(processed_data);
			
			csv_data = odv_clean_input(csv_data);
			
			$('#processed_csvdata').val(csv_data);
		}
		prev_strep = 'preview-csv';
	});
	
	$("#confirm-process").click(function() {
		var title = odv_clean_input($('#dataset-title').val());
		var desc = odv_clean_input($('#dataset-desc').val());
		var email = odv_clean_input($('#email').val());
		
		$.cookie('dataset_title', title);
		$.cookie('dataset_desc', desc);
		$.cookie('dataset_email', email);
		prev_strep = 'confirm-process';
	});
	
});

function selectHeaderType(ele) {
	var data_type = $(ele).val();
	$(ele).parent().find('.header-meta').hide();
	console.log(data_type);
	switch(data_type) {
		case 'datetime':
			$(ele).parent().find('.header-meta').show();
			break;
	}	
}

function transposeData() {
	if(typeof(csv_arr[0]) === 'undefined') {
	    return null;
	  }
	csv_arr = transpose(csv_arr);
	reGeneratedata(csv_arr);
}
function reGeneratedata(csv_arr) {
	var header_data = getHeaderData(csv_arr);
	var html = generateTable(csv_arr);
	
	html = odv_clean_input(html);
	
	$('#csvarray').html('');
	$('#csvarray').html(html);
	$('#dataColumn').html('');
	for (x in header_data){
		var input = '<div class="header-column-content"><input class="header-column span11" name="header-' + x + '" value="' + header_data[x] + '" />' +
					'<select class="header-type span6" onChange="selectHeaderType(this)" ><option value="none">Data Type</option>'+
					'<option value="number">Number</option>'+
					'<option value="string">String</option>'+
					'<option value="datetime">Date Time</option></select>' +
					'<select class="header-meta datetime span6">'+
					'<option value="dd/mm/yyyy">dd/mm/yyyy</option>'+
					'<option value="mm/dd/yyyy">mm/dd/yyyy</option>'+
					'<option value="mm-dd-yyyy">mm-dd-yyyy</option>'+
					'<option value="dd-mm-yyyy">dd-mm-yyyy</option></select>' +
					'<div class="span4"><input type="checkbox" class="disable-header" id="hide-' + x + '" value="' + x + '">'+
					'<label for="hide-' + x + '">Drop</label><div class="clear-fix"></div><hr/></div></div>';
		
		input = odv_clean_input(input);
		
		$('#dataColumn').append(input);
	}
}
function generatCSV(data) {
	  var csv = '';

	  if(typeof(data[0]) === 'undefined') {
	    return null;
	  }

	  if(data[0].constructor === String) {
	    for(var item in data) {
	    	csv += data[item] + ',';
	    }
	    csv += '\r\n';
	  }

	  if(data[0].constructor === Array) {
	    for(var row in data) {
	      for(var item in data[row]) {
	    	  csv += data[row][item] + ',';
	      }
	      csv = csv.replace(/(\s+)?.$/, '');
	      csv += '\r\n';
	    }
	  }
	  
	  return csv;
}

function processHeader(data) {
	if(typeof(data[0]) === 'undefined') {
	    return null;
	  }
	var headercolumn = [];
	var headertype = [];
	var headermeta = {
			number:[],
			string:[],
			datetime:[]
	};
	
	var removeheader = [];
	$('.header-column').each(function(i){
		headercolumn.push($(this).val());
	});
	$('.header-type').each(function(i){
		var val = $(this).val();
		headertype.push(val);
		if (val !='string' && val !='none') headercolumn[i] += " as " + val;
	});
	
	$('.header-meta').each(function(i){
		if($(this).hasClass('datetime')) {
			var val = $(this).val();
			headermeta.datetime.push(val);
		}
	});
	
	data[0] = headercolumn;
	$('.disable-header:checked').each(function(i){
		removeheader.push($(this).val());
	});
	data = transpose(data);
	$.each(removeheader,function(i, l){
		data = removeKey(data,l);
	});
	
	data = transpose(data);
	
	data = proecessData(data,headercolumn, headertype, headermeta);
	
	return data;
}

function proecessData(data,headercolumn, headertype, headermeta) {
	data = transpose(data);
	
	for(var i = 0; i < data.length; i++) {
		if(headertype[i] == 'number') {
			if(data[i] == 'NA' || data[i] == 'na' || data[i] == '') { //NA to Null
				data[i] = null;
			}
		} else if(headertype[i] == 'datetime') {
			data[i] = processRowToDateTime(data[i], headermeta.datetime[i]);
			console.log(data[i]);
		}
	}
	
	return transpose(data);
}

function padStr(i) {
    return (i < 10) ? "0" + i : "" + i;
}

/*
 * dd/mm/yyyy to Date object
 */
function processRowToDateTime(row, format) {
	for(var i = 1; i < row.length; i ++) {
		if(row[i] == '') {
			row[i] = null;
			continue;
		}
		switch(format) {
			case 'dd-mm-yyyy':
				var reggie = /(\d{2})-(\d{2})-(\d{4})/;
				var dateArray = reggie.exec(row[i]);  
				var dateObject = new Date(
				    (+dateArray[3]),
				    (+dateArray[2])-1, // Careful, month starts at 0!
				    (+dateArray[1])
				);
				break;
			case 'mm-dd-yyyy':
				var reggie = /(\d{2})-(\d{2})-(\d{4})/;
				var dateArray = reggie.exec(row[i]); 
				var dateObject = new Date(
				    (+dateArray[3]),
				    (+dateArray[1])-1, // Careful, month starts at 0!
				    (+dateArray[2])
				);
				break;
			case 'mm/dd/yyyy':
				var reggie = /(\d{2})\/(\d{2})\/(\d{4})/;
				var dateArray = reggie.exec(row[i]); 
				var dateObject = new Date(
				    (+dateArray[3]),
				    (+dateArray[1])-1, // Careful, month starts at 0!
				    (+dateArray[2])
				);
				break;
			default: // dd/mm/yyyy
				var reggie = /(\d{2})\/(\d{2})\/(\d{4})/;
				var dateArray = reggie.exec(row[i]); 
				var dateObject = new Date(
				    (+dateArray[3]),
				    (+dateArray[2])-1, // Careful, month starts at 0!
				    (+dateArray[1])
				);
				break;
		}
		
		row[i] = dateObject.getFullYear() + "-" + padStr((dateObject.getMonth() + 1)) + "-" + padStr(dateObject.getDate()) + " " +
		padStr(dateObject.getHours()) + ":" + padStr(dateObject.getMinutes()) + ":" + padStr(dateObject.getSeconds());
		
	}
	
	return row;
}

function getHeaderData(data){
	if(typeof(data[0]) === 'undefined') {
	    return null;
	  }
	else{
		return data[0];
	}
}
//build HTML table data from an array (one or two dimensional)
function generateTable(data) {
  var html = '<table class="table table-striped"><caption>Preview Data</caption>';
  var row_count = 10;
  if(typeof(data[0]) === 'undefined') {
    return null;
  }

  if(data[0].constructor === String) {
    html += '<tr>\r\n';
    for(var item in data) {
      html += '<td>' + data[item] + '</td>\r\n';
    }
    html += '</tr>\r\n';
  }

  if(data[0].constructor === Array) {
	var i = 0; 
    for(var row in data) {
      if(i < row_count) {
	      html += '<tr>\r\n';
	      for(var item in data[row]) {
	        html += '<td>' + data[row][item] + '</td>\r\n';
	      }
	      html += '</tr>\r\n';
      } else {
    	  break;
      }
      i++;
    }
    
  }

  if(data[0].constructor === Object) {
    for(var row in data) {
      html += '<tr>\r\n';
      for(var item in data[row]) {
        html += '<td>' + item + ':' + data[row][item] + '</td>\r\n';
      }
      html += '</tr>\r\n';
    }
  }
  html += "</table>";
  return html;
}
 
function transpose(a)
{
  return Object.keys(a[0]).map(function (c) { return a.map(function (r) { return r[c]; }); });
}

function removeKey(arrayName,key)
{
 var x;
 var tmpArray = new Array();
 for(x in arrayName)
 {
  if(x!=key) { tmpArray[x] = arrayName[x]; }
 }
 return tmpArray;
}