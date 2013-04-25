var csv_arr = {};
var processed_data = {};
$(function(){
	$("#proces-csv").click(function() {
		if ($('#csv-data').val() != '') {
			var csv = $('#csv-data').val();
			csv_arr = $.csv.toArrays(csv);
			reGeneratedata(csv_arr);
		}
		});
	$("#preview-csv").click(function() {
		processed_data = processHeader(csv_arr);
		if(processed_data == null) {
		    return null;
		  }
		$('#preview-data').html('');
		var new_data = generateTable(processed_data);
		$('#preview-data').html(new_data);
		var csv_data = generatCSV(processed_data);
		$('#processed_csvdata').val(csv_data);
	});
	$("#confirm-process").click(function() {
		var title = $('#dataset-title').val();
		var desc = $('#dataset-desc').val();
		var email = $('#email').val();
		$.cookie('dataset_title', title);
		$.cookie('dataset_desc', desc);
		$.cookie('dataset_email', email);
	});
});
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
	$('#csvarray').html('');
	$('#csvarray').html(html);
	$('#dataColumn').html('');
	for (x in header_data){
		var input = '<input class="header-column span11" name="header-' + x + '" value="' + header_data[x] + '" />' +
					'<select class="header-type span6"><option value="none">Data Type</option><option value="number">Number</option><option value="string">String</option><option value="datetime">Date Time</option></select>' +
					'<div class="span4"><input type="checkbox" class="disable-header" id="hide-' + x + '" value="' + x + '"><label for="hide-' + x + '">Drop</label></div>';
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
	var removeheader = [];
	$('.header-column').each(function(i){
		headercolumn.push($(this).val());
	});
	$('.header-type').each(function(i){
		var val = $(this).val();
		headertype.push(val);
		if (val !='string' && val !='none') headercolumn[i] += " as " + val;
	});
	data[0] = headercolumn;
	$('.disable-header:checked').each(function(i){
		removeheader.push($(this).val());
	});
	data = transpose(data);
	$.each(removeheader,function(i, l){
		data = removeKey(data,l);
	});

	return transpose(data);
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
    for(var row in data) {
      html += '<tr>\r\n';
      for(var item in data[row]) {
        html += '<td>' + data[row][item] + '</td>\r\n';
      }
      html += '</tr>\r\n';
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

function removeKey(arrayName,key) {
 var x;
 var tmpArray = new Array();
 for(x in arrayName) {
  if(x!=key) { 
	  tmpArray[x] = arrayName[x]; 
  }
 }
 return tmpArray;
}
