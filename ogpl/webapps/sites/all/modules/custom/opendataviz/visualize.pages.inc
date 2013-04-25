<?php

function file_get_contents_curl($url) {
  $ch = curl_init();
   
  curl_setopt($ch, CURLOPT_HEADER, 0);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); //Set curl to return the data instead of printing it to the browser.
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
  
  $data = curl_exec($ch);
  curl_close($ch);
   
  return $data;
}

function opendataviz_visualization_instance($node) {
  global $base_url;
  $header = array(
      array('data' => t('Title'), 'field' => 'title', 'sort' => 'desc'),
      array('data' => t('Instance'), 'field' => 'data_instance'),
      array('data' => t('Status'), 'field' => 'active'),
      array('data' => t('Operations'))
  );
  
  $sql = 'SELECT id, title, active, data_instance FROM {opendataviz_instance} WHERE node_id='.$node->nid . tablesort_sql($header);
  
  $result = pager_query($sql, 30);
  $rows = array();
  
  while ($log = db_fetch_object($result)) {
    $rows[] = array(
        array('data' => t($log->title), 'class' => 'nowrap'),
        $log->data_instance,
        $log->active,
        l(t('View'), $base_url.'/visualize.php?inst=' . $log->data_instance . '&nid='.$node->nid, array('attributes' => array('class' => 'preview-link', 'target' => '_blank'))) . 
        //l('edit', '', array('attributes' => array('class'=>'edit-link'))) .  
        l(t('Delete'),'deletevisualizationinstance/' . $log->id,array('attributes'=>array('class'=>'delete-link Delete-visualization'), 'query'=> drupal_get_destination())) .
        ($log->active?l(t('Deactive'),'disapprovevisualizationinstance/' . $log->id,array('attributes'=>array('class'=>'Disapprove-visualization'), 'query'=> drupal_get_destination())): l(t('Activate'),'approvevisualizationinstance/' . 
        $log->id,array('attributes' => array('class'=>'Approve-visualization'), 'query'=> drupal_get_destination()))) 
    );
  }
  
  if (empty($rows)) {
    $rows[] = array(array('data' => t('No Instance available.Please add your first instance ') . l('here','node/'.$node->nid.'/createvisualization'), 'colspan' => 4));
  }
  else {
    $rows[] = array(array('data' => t('') . l('Add More Instance','node/'.$node->nid.'/createvisualization'), 'colspan' => 4));
  }
  
  $output = theme('table', $h2eader, $rows);
  $output .= theme('pager', NULL, 30, 0);
  
  return $output;
}

function _save_opendata_instance($post_data, $nid) {
  global $user;
  
  if(function_exists('web_misc_script_remove_blacklist')) {
    $post_data['csvdata']      = web_misc_script_remove_blacklist($post_data['csvdata']);
    $post_data['dataset-tite'] = web_misc_script_remove_blacklist($post_data['dataset-tite']);
    $post_data['dataset-desc'] = web_misc_script_remove_blacklist($post_data['dataset-desc']);
  }
  
  $csv_string = check_plain($post_data['csvdata']);
  $title = check_plain($post_data['dataset-tite']);
  $desc = check_plain($post_data['dataset-desc']);
  $instance = md5( microtime() . $nid);
  
  
  
  $data = array(
      'user_id' => $user->uid,
      'node_id' => $nid,
      'data_instance' => $instance,
      'title' => $title,
      'description' => $desc,
      'data' => $csv_string,
      'date_created' => time()
  );
  if (drupal_write_record('opendataviz_instance', $data)) {
    return $instance;
  }
  
  return FALSE;
}
function opendataviz_get_worksheets_ajax_callback($file, $type='application/vnd.ms-excel') {
  require_once 'sites/all/modules/custom/dms_ds_upload/Classes/PHPExcel/IOFactory.php';
  $inputFileName = $file;
  if (!is_array($inputFileName)) {
    if ( in_array($type, array('application/vnd.ms-excel', 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'))) {
      $objPHPExcel = PHPExcel_IOFactory::load($inputFileName);
      $loadedSheetNames = $objPHPExcel->getSheetNames();
      $json_obj = new stdClass();
      $html = "<ul>";
      foreach($loadedSheetNames as $sheetIndex => $loadedSheetName) {
        $sheetData = $objPHPExcel->getSheet($sheetIndex)->toArray(null,true,true,true);
        if(count($sheetData) > 1){
          $url = $_SERVER['REQUEST_URI'];
          $html .= '<li><a href="?worksheet=' . $sheetIndex . '">' . $loadedSheetName . '</a></li>';
        }
      }
      $html .= "</ul>";
      $json_obj->msg = $html;
      return $html;
    }
    else{
      $json_obj = new stdClass();
      $json_obj->msg = "No worksheet";
      $html = "No worksheet";
      return $html;
    }
  }
  else {
    return "Invalid file";
  }
}

function opendataviz_visualize($node) {
  include drupal_get_path('module', 'opendataviz')."/lib/datatool/getCSVData.php";
  if($_POST['csvdata']) {
    if ($instance = _save_opendata_instance($_POST,$node->nid)) {
      drupal_set_message(t("Data Visualization is ready to use. Please visit"));
      drupal_goto('node/'.$node->nid."/visualizationinstance");
    }
  }
  global $base_url;
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/lib/bootstrap/css/bootstrap.css');
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/css/slidingform.css');
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/lib/dataviz/css/style.css');
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/lib/colorpicker/css/colorpicker.css');
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/lib/graphicons/graphicons.css');
  
  /* add required js files */
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/jquery/jquery-1.8.2.min.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/jquery/jquery.csv.min.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/bootstrap/js/bootstrap.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/colorpicker/js/bootstrap-colorpicker.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/js/sliding.form.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/js/jquery.cookie.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/js/script.js', 'module');
  
  $file_data = $node->field_dsat_downloadable[0]['value']['field_dam_datafile'];
  
  switch($node->field_dsat_access_method[0][value]) {
    case 'Downloadable':
      $file_mime = $file_data[0]['filemime'];
      
      switch (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC)) {
        case FILE_DOWNLOADS_PUBLIC:
          $fileurl = $base_url . "/" . $file_data[0]['filepath'];
          break;
        case FILE_DOWNLOADS_PRIVATE:
          $fileurl = $base_url . str_replace(file_directory_path(), '/system/files', $file_data[0]['filepath']);
          break;
      }
      
      if ($file_mime == 'text/csv') {
        $data = file_get_contents_curl($fileurl);
      }
      else {
        $worksheet_name = opendataviz_get_worksheets_ajax_callback($file_data[0]['filepath']);
        if (!$_GET['worksheet']) { $worksheet = 0; } else { $worksheet = $_GET['worksheet']; }
        $csv_convert = new getCSVData();
        $data = $csv_convert->getCSV($file_data[0]['filepath'], $worksheet, $file_mime);
      }
      break;
    case 'Link':
      $fileurl = $node->field_dsat_access_point[0]['value']['field_elam_access_point'][0]['url'];
      $format_tid = $node->field_dsat_access_point[0]['value']['field_elam_file_format'][0]['value'];
      $term = taxonomy_get_term($format_tid);
      $file_mime = $term->name;
      if ($file_mime == 'text/csv') {
        $data = file_get_contents_curl($fileurl);
      }
      break;
  }
  
  $output = '<div class="container-fluid container-main">
  	<div class="row-fluid">
  			<div class="span12">
  			    <div class="row-fluid">
            			<div id="content-visualize">
                      <div id="wrapper">
                              <div id="navigation" style="display:none;">
                               <ul>
                                  <li class="selected step1">
                                      <a href="#" id="csv-source">Paste Your CSV File</a>
                                  </li>
                                  <li class="step2">
                                      <a href="#" id="proces-csv">Process CSV</a>
                                  </li>
                                  <li class="step3">
                                      <a href="#" id="preview-csv">Preview Data</a>
                                  </li>
                                  <li class="step4">
                                      <a href="#" id="account-details">Instance Details</a>
                                  </li>
                                  <li class="step5">
                                      <a href="#" id="confirm-process">Confirm</a>
                                  </li>
                              </ul>
                      </div>
                      <div id="steps">
                          <form id="formElem" name="formElem" action="" method="post">
                              <fieldset class="step">
                                  <legend>Data Source</legend>
                                      <div id="data-source-wrapper">
                                       Choose Worksheet: ' . $worksheet_name . '
                                           <br/>
                                      <textarea rows="20" class="span12" id="csv-data">' . $data . '</textarea>
                                      </div> 
                                      <div class="row-fluid">
                                        <div class="span12">
                                          <div class="show_process_progress"></div>
                                        </div>
                                      </div>   
                              </fieldset>
                              <fieldset class="step">
                                  <legend>Process CSV</legend>
                                      <div id="process-control">
                                        <div class="row-fluid">
                                          <div class="span9">
                                            <div id="csvarray" class="well well-small">No Data Found</div>
                                          </div>
                                          <div class="span3">
                                            <div class="well well-small process-csv-control">
                                              <div class="label label-succes">Change Column Properties</div>
                                              <div id="dataColumn"></div>
                                              <hr>
                                              <div class="btn-group" data-toggle="buttons-checkbox">
                                                    <button type="button" class="btn btn-info" onclick="transposeData();">Transpose Data</button>
                                              </div>
                                            </div>
                                          </div>
                                        </div>
                                        <div class="row-fluid">
                                          <div class="span12">
                                            <div class="show_process_progress"></div>
                                          </div>
                                        </div>
                                      </div>
                              </fieldset>
                              <fieldset class="step">
                                  <legend>Preview Data</legend>
                                      <div id="preview-control">
                                        <div class="row-fluid">
                                          <div class="span12" id="preview-data"></div>
                                        </div>
                                      </div>
                              </fieldset>
                              <fieldset class="step">
                                <legend>Dataset Instance Details</legend>
                              <div id="dataset-details-wrapper">
                                <p>
                                    <label for="dataset-title">Title of Dataset</label>
                                    <input id="dataset-title" name="dataset-tite" />
                                </p>
                                <p>
                                    <label for="dataset-desc">Dataset Description</label>
                                    <textarea id="dataset-desc" name="dataset-desc"></textarea>
                                </p>
                             </div>
                                
                        </fieldset>
      						<fieldset class="step">
                                  <legend>Confirm</legend>
                    <div class="confirm-wrapper">                      
      							<p>
      								Everything in the form was correctly filled 
      								if all the steps have a green checkmark icon.
      								A red checkmark icon indicates that some field 
      								is missing or filled out with invalid data. In this
      								last step the user can confirm the submission of
      								the form.
      							</p>
                                  <p class="submit">
                                      <input type="hidden" name="csvdata" id="processed_csvdata" value="" />
                                      <button id="registerButton" type="submit">Proceed to Visualize</button>
                                  </p>
                    </div>                      
                              </fieldset>
                          </form>
                      </div>
                  </div>
              </div>
            </div>
  			</div>
  	</div>
  </div>';
  return $output;
}

/**
 * Json Outout for the csv data
 */
function opendataviz_json_output($inst) {
  #ob_start("ob_gzhandler");
  require_once drupal_get_path('module', 'opendataviz')."/lib/queryengine/vistable.php";
  global $base_url;
  $tqx = isset($_GET['tqx']) ? $_GET['tqx'] : "";
  $tq = isset($_GET['tq']) ? $_GET['tq'] : "";
  $tqrt = isset($_GET['tqrt']) ? $_GET['tqrt'] : "";
  $tz = isset($_GET['tz']) ? $_GET['tz'] : "PDT";
  //$inst = isset($_GET['inst']) ? check_plain($_GET['inst']) : "";
  $locale = isset($_SERVER["HTTP_ACCEPT_LANGUAGE"]) ? $_SERVER["HTTP_ACCEPT_LANGUAGE"] : "en_US";

  if (isset($_GET['locale'])) {
    $locale = $_GET['locale'];
  }

  $extra = array();
  if (isset($_GET['debug']) && $_GET['debug']) {
    header('Content-type: text/plain; charset="UTF-8"');
    $extra['debug'] = 1;
  }

  $vt = new csv_vistable($tqx, $tq, $tqrt,$tz,$locale,$extra);
  if ($inst) {
    $result = db_query("SELECT * FROM {opendataviz_instance} WHERE data_instance = '%s'", $inst);
    while ($row = db_fetch_object($result)) {
      $data = $row->data;
      $vt->setup_table($data);
      $result = $vt->execute();
      echo $result;
    }
  }
  else {
    die("Invalid Instance");
  }
  #ob_end_flush();

}

function opendataviz_visualize_output($inst) {
  global $base_url;
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/lib/bootstrap/css/bootstrap.css');
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/lib/colorpicker/css/colorpicker.css');
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/lib/dataviz/css/style.css');
  drupal_add_css(drupal_get_path('module', 'opendataviz') . '/lib/graphicons/graphicons.css');
  
  /*
  $external_css = array('http://serverapi.arcgisonline.com/jsapi/arcgis/3.2/js/dojo/dijit/themes/claro/claro.css','http://serverapi.arcgisonline.com/jsapi/arcgis/3.2/js/esri/css/esri.css');
  
  foreach ($external_css as $css_value) {
    drupal_add_css('document.write(unescape("%3Clink rel="stylesheet" href=\''. urlencode($css_value) . '\' type=\'text/css\' /%3E"));', 'inline', 'header');
  }
  */
  
  /* add required js files */
  //drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/jquery/jquery-1.8.2.min.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/jquery/jquery.csv.min.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/bootstrap/js/bootstrap.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/colorpicker/js/bootstrap-colorpicker.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/dataviz/init.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/lib/dataviz/query_builder.js', 'module');
  drupal_add_js(drupal_get_path('module', 'opendataviz') . '/js/script.js', 'module');
  /*
  $external_js = array('http://maps.google.com/maps/api/js?key=AIzaSyDrq5xE0v4wqyElRaIHzr9xUql8OQpFWAQ%26region=IN%26sensor=true','http://www.google.com/jsapi','http://serverapi.arcgisonline.com/jsapi/arcgis/?v=3.2','http://serverapi.arcgisonline.com/jsapi/gmaps/?v=1.6');
  
  foreach ($external_js as $js_value) {
  drupal_add_js('document.write(unescape("%3Cscript src=\''. $js_value . '\' type=\'text/javascript\'%3E%3C/script%3E"));', 'inline', 'header');
  }
  */
  $query_instance = $base_url."/opendatavisualization/" . $inst . "/json";
  /*
  drupal_add_js("google.load('visualization', '1', {packages: ['corechart', 'table', 'geomap', 'motionchart']});
      DataViz.query_instance = '$query_instance' ;
      google.setOnLoadCallback(DataViz.drawVisualization);
	  
      function initialize() {
        var mapDiv = document.getElementById('data_viz_map');
        var map = new google.maps.Map(mapDiv, {
          center: new google.maps.LatLng(37.4419, -122.1419),
          zoom: 13,
          mapTypeId: google.maps.MapTypeId.ROADMAP
        });
      
      }
      google.maps.event.addDomListener(window, 'load', initialize);",'inline', 'header');
      
      */
  $output = '<div class="container-fluid">
		<div class="row-fluid">
      <div class="span12">
      <div class="graphicon-circle-info pull-right" onclick="DataViz.toggleDatasetInfo();"></div>
      <div class="clearfix"></div>
        <div class="dataset-details alert alert-info" style="display:none;">
            <h2 id="dataset-title"></h2><small><span id="dataset-email"></span></small>
            <p id="dataset-desc"></p>
          </div>
      </div>
    </div>
		<div class="row-fluid">
			<div class="span12">
				<!--Sidebar content-->
				<!-- 
		    	<form class="navbar-search pull-right">
			    	<input type="text" class="search-query" placeholder="Search">
			    </form>
				-->
				<div id="data_component" class="btn-group pull-right" data-toggle="buttons-checkbox">
				    <button type="button" title="Grid" class="btn grid active">
						<span class="graphicon-grid"></span>
					</button>
				    <button type="button" title="Chart" class="btn chart">
						<span class="graphicon-chart"></span>
					</button>
				  <button type="button" title="Map" class="btn map">
						<span class="graphicon-map"></span>
					</button>
					<button type="button" title="NIC Map" class="btn arcgismap">
            <span class="graphicon-map"></span>
          </button>
		    	</div>
			    <div class="clearfix"></div>
				<div class="record-count"></div>
			</div>
		</div>
	    <div class="row-fluid">
		    <div class="span8 well">
		    	<!--Body content-->
		    	<div id="data_viz_grid" style="height: 400px; width:100%"><center><img src="images/loading.gif" /></center></div>
		    	<div id="data_viz_chart" style="height: 400px;">    
			    	<div class="alert alert-block">
					    <button type="button" class="close" data-dismiss="alert">×</button>
					    <h4>Enjoy Visualization!</h4>
					    Select the chart axis to generate your chart...
					  </div>
				  </div>
		    	<div id="data_viz_geomap" style=""></div>
		    	<div id="data_viz_map" style="height: 400px;"></div>
		    	<div id="data_viz_arcgismap" style="height: 400px;"></div>
		    	<div id="data_viz_timeline" style=""></div>
				
		    </div>
		    <div class="span4 well">
				<div  class="accordion" id="accordion2">
					<div class="accordion-group">
						<div class="accordion-heading">
							<a class="accordion-toggle" data-toggle="collapse" data-parent="#accordion2" href="#collapseOne">
							Fields
							</a>
						</div>
						<div id="collapseOne" class="accordion-body collapse out">
							<div class="accordion-inner">
								<div  id="data_control_pane"></div>
							</div>
						</div>
					</div>
					<div class="accordion-group">
						<div class="accordion-heading">
							<a class="accordion-toggle" data-toggle="collapse" data-parent="#accordion2" href="#collapseTwo">
							Filter
							</a>
						</div>
						<div id="collapseTwo" class="accordion-body collapse out">
							<div class="accordion-inner">
								<div id="data_viz_filter_form">
									<form>
										<select id="data_viz_and_or">
											<option value="and">and</option>
											<option value="or">or</option>
										</select>
										<select id="data_viz_available_field" class="data_viz_available_field" onchange="DataViz._populat_operator(this.value)">
											<option value="">Select Field</option>
										</select>
										<select id="data_viz_operator">
											<option value="">Select Operator</option>
										</select>
										<input id="data_viz_filter_value" type="text"/>
										<div></div>
										<button type="button" class="btn" onclick="DataViz.addFilter()" >Add</button>
									</form>
								</div>
								<div id="data_viz_filter">
									<div class="row-fluid">
								      <div class="span12"><H6>Applied Filter</H6></div>
								    </div>
								</div>
							</div>
						</div>
					</div>
					<div class="accordion-group">
						<div class="accordion-heading">
							<a class="accordion-toggle" data-toggle="collapse" data-parent="#accordion2" href="#collapseThree">
							Groups
							</a>
						</div>
						<div id="collapseThree" class="accordion-body collapse out">
							<div class="accordion-inner">
								<div  id="data_control_pane_group">
									<select id="data_viz_available_groupby_data_field" class="data_viz_available_field" >
										<option value="">Select Group</option>
									</select>
									<button type="button" class="btn" onclick="DataViz.addGroup()">Add</button>
									<div id="data_viz_group"></div>
								</div>
							</div>
						</div>
					</div>
					<div class="accordion-group">
						<div class="accordion-heading">
							<a class="accordion-toggle" data-toggle="collapse" data-parent="#accordion2" href="#collapseFour">
							Visualize Chart
							</a>
						</div>
						<div id="collapseFour" class="accordion-body collapse out">
							<div class="accordion-inner">
							  <div class="span6">
									<div id="data_control_pane_chart"></div>
									<div id="data_chart_axis">
										<form id="data_viz_chart_form" method="post" onsubmit="return DataViz.generateGraph(this);" >
											<select name="base" id="data_viz_available_chart_base_field" class="data_viz_available_field span11" onchange="DataViz.setBaseLabel(this.value)">
												<option value="">Select Base Axis</option>
											</select>
											<select id="data_viz_available_chart_data_field" class="data_viz_available_field span11" >
												<option value="">Select Series</option>
											</select>
											<button type="button" class="btn" onclick="DataViz.addSeries()">Add</button>
											<div id="data_viz_added_series">
											</div>
											<!-- <input type="submit" class="btn" value="Generate"></input> -->
										</form>
									</div>
								</div>
								<div class="span6">
									<input id="chart_name" type="text" placeholder="Name of Your Chart" class="span11">
									<input id="chart_base_axis_label" type="text" placeholder="Base Axis Label" class="span11">
									<input id="chart_series_axis_label" type="text" placeholder="Series Label" class="span11">
								</div>
							</div>
						</div>
					</div>
					<div class="accordion-group">
            <div class="accordion-heading">
              <a class="accordion-toggle" data-toggle="collapse" data-parent="#accordion2" href="#collapseFive">
              MAP
              </a>
            </div>
            <div id="collapseFive" class="accordion-body collapse out">
              <div class="accordion-inner">
                <div id="data_map">
                  <form>
                    <select name="base" id="data_viz_available_map_lat_field" class="data_viz_available_field" >
                      <option value="">Select Latitude</option>
                    </select>
                    <select id="data_viz_available_map_lon_field" class="data_viz_available_field" >
                      <option value="">Select Longitude</option>
                    </select>
                    <button type="button" class="btn" onclick="DataViz.generateMap()">Generate</button>
                  </form>
                </div>
              </div>
            </div>
          </div>
					 <div class="accordion-group">
            <div class="accordion-heading">
              <a class="accordion-toggle" data-toggle="collapse" data-parent="#accordion2" href="#collapseSix">
              Custom Query
              </a>
            </div>
            <div id="collapseSix" class="accordion-body collapse out">
              <div class="accordion-inner">
                <div  id="data_control_pane_group">
                 <form>
                    <textarea id="data_viz_custom_query_value"></textarea>
                    <button type="button" class="btn" onclick="DataViz.executeQuery()" >Execute</button>
                    <div class="clearfix"></div>
                    <div class="label label-inverse">For query language help visit <br/><a href="https://developers.google.com/chart/interactive/docs/querylanguage" target="_blank">Query Language Reference</a></div>
                  </form>
                </div>
              </div>
            </div>
          </div>
				</div>
			</div>
	    </div>
    </div>';
  
  return $output;
}

/**
 * Menu callback -- ask for confirmation of node Approve
 */
function approve_visualization_instance(&$form_state, $instance_id) {
  if(is_numeric($instance_id)) {
    $data = _get_visualization_instance($instance_id);
    return confirm_form($form,
        t('Are you sure you want to approve %title for visualization?', array('%title' => $data->title)),
        isset($_GET['destination']) ? $_GET['destination'] : '<front>',
        t(''),
        t('Approve'),
        t('Cancel')
    );
  }
}

/**
 * Menu callback -- ask for confirmation of node Disapprove
 */
function disapprove_visualization_instance(&$form_state, $instance_id) {
   if(is_numeric($instance_id)) {
    $data = _get_visualization_instance($instance_id);
    return confirm_form($form,
        t('Are you sure you want to Disapprove %title for visualization?', array('%title' => $data->title)),
        isset($_GET['destination']) ? $_GET['destination'] : '<front>',
        t(''),
        t('Disapprove'),
        t('Cancel')
    );
   }
}

/**
 * Menu callback -- ask for confirmation of node Delete
 */
function delete_visualization_instance(&$form_state, $instance_id) {
  if(is_numeric($instance_id)) {
    $data = _get_visualization_instance($instance_id);
    return confirm_form($form,
        t('Are you sure you want to delete %title ?', array('%title' => $data->title)),
        isset($_GET['destination']) ? $_GET['destination'] : '<front>',
        t(''),
        t('Delete'),
        t('Cancel')
    );
  }
}

/**
 * Execute visualization Approve
 */
function approve_visualization_instance_submit($form, &$form_state) {
  $token  = drupal_get_token($form['#post']['form_id']);
  if($token == $form['#post']['form_token']) {
    if ($form_state['values']['confirm']) {
      $instance_id = check_plain(arg(1));
      $data = array(
          'id' => $instance_id,
          'active' => 1
      );
      if(_update_visualization_instance($data)) 
      {
        drupal_set_message('Instance Activated');
      }
    }
  } else {
    drupal_set_message('Invalid operation!');
  }
  $form_state['redirect'] = drupal_get_destination();
}

/**
 * Execute visualization Disapprove
 */
function disapprove_visualization_instance_submit($form, &$form_state) {
  $token  = drupal_get_token($form['#post']['form_id']);
  if($token == $form['#post']['form_token']) {
    if ($form_state['values']['confirm']) {
      $instance_id = check_plain(arg(1));
      $data = array(
          'id' => $instance_id,
          'active' => 0
      );
      if(_update_visualization_instance($data)) 
      {
        drupal_set_message('Instance Deactivated');
      }
    }
  } else {
    drupal_set_message('Invalid operation!');
  }
  $form_state['redirect'] = drupal_get_destination();
}

/**
 * Execute visualization Delete
 */
function delete_visualization_instance_submit($form, &$form_state) {
  $token  = drupal_get_token($form['#post']['form_id']);
  if($token == $form['#post']['form_token']) {
    if ($form_state['values']['confirm']) {
      $instance_id = check_plain(arg(1));
      $data = array(
          'id' => $instance_id,
          'active' => 0
      );
      if(_delete_visualization_instance($data))
      {
        drupal_set_message('Instance Deleted');
      }
    }
  } else {
    drupal_set_message('Invalid operation!');
  }
  $form_state['redirect'] = drupal_get_destination();
}



function _get_visualization_instance($instance_id) {
  $result = db_query("SELECT * FROM {opendataviz_instance} WHERE id = %d", $instance_id);
  if($row = db_fetch_object($result)) {
    return $row;
  }
  return NULL;
}

function _update_visualization_instance($data) {
  if (drupal_write_record('opendataviz_instance', $data, array('id'))) {
    return TRUE;
  }
  return FALSE;
}

function _delete_visualization_instance($data) {
  if ($result = db_query("DELETE FROM {opendataviz_instance_view} WHERE data_instance_id = %d", $data[id])) {
    if ($result = db_query("DELETE FROM {opendataviz_instance} WHERE id = %d", $data[id]))
      return TRUE;
  }
  return FALSE;
}

function opendataviz_instance_list($node) {
  global $user;
  
  $auth = FALSE;
  
  $roles = array(
      'Administrator',
      'PMO',
      'PMU',
      'Validator/Approver (Data Controller)'
  );
  
  foreach($user->roles as $role) {
    if(in_array($role, $roles)) {
      $auth = TRUE;
    }
  }
  
  if($auth) {
    $result = db_query("SELECT id, title, data_instance FROM {opendataviz_instance} WHERE node_id = %d AND user_id = %d ", $node->nid, $user->uid);
  } else {
    $result = db_query("SELECT id, title, data_instance FROM {opendataviz_instance} WHERE node_id = %d AND active = 1 ", $node->nid);
  }
  
  $data = array(
      'status' => FALSE,
      'message' => 'No result Found!',
      'data'=> NUll
  );
  
  if($result) {
    $rows = array();
    while ($row = db_fetch_object($result)) {
      $rows[$row->id] = $row; 
    }
    
    if(count($rows)) {
      $data = array(
          'status'   => TRUE,
          'message'  => 'ok',
          'data'=> $rows
      );
    }
  }
  
  print json_encode($data);
  die();
}


function opendataviz_node_detail($node) {
  if ($node) {
    $frontend_url = variable_get('frontend_url', '');
    $url = $frontend_url . '/' . url('node/' . $node->nid);
    $detail = array(
        'nid'         => $node->nid,
        'url'         => $url,
        'title'       => $node->title,
        'description' => $node->field_ds_description[0]['value'],
        'note'        => $node->field_support_note[0]['value']
    );
    
    $data = array(
        'status'   => TRUE,
        'message'  => 'ok',
        'data' => $detail
    );
  } else {
    $data = array(
      'status'   => FALSE,
      'message'  => 'Invalid node..'
    );
  }
  print json_encode($data);
  die();
}

function _get_visualization_instance_by_key($key) {
  if($safe_key = check_plain($key)) {
    $result = db_query("SELECT * FROM {opendataviz_instance} WHERE data_instance = '%s'", $safe_key);
    if($row = db_fetch_object($result)) {
      return $row;
    }
  }
  return NULL;
}

function opendataviz_view_json_output($key) {
  if ($key != '') {
    if ($result = _get_visualization_instance_by_key($key)) {
      $node = node_load($result->node_id);
      
      $view_result = _get_visualization_instance_view_by_keyid($result->id, $node);
      
      $operation = opendataviz_view_check_operation($result->user_id, TRUE);
      
      $data = array(
        'status'   => TRUE,
        'message'  => 'ok',
        'title'    => 'Views',
        'data'     => $view_result, 
        'html'     => '',
        'operation'=> $operation
          
      );
      
    }
    else {
      $data = array(
          'status' => FALSE,
          'message' => 'No result Found!',
          'title'    => '' ,
          'html'=>''
      );
    }
  }
  else {
    $data = array(
        'status' => FALSE,
        'message' => 'No result Found!',
        'html'=>''
    );
  }
  
  print json_encode($data);
  die();
}

function opendataviz_instance_view_update($key) {
  $input_data = $_POST['data'];
  $key     = $_POST['key'];
  $id      = $_POST['id'];
  $title   = $_POST['title'];
  
  $data = array(
      'status'  => FALSE,
      'message' => 'Invalid operation',
  );
  
  if($view = _get_visualization_instance_view_by_id($id)) {
    $opration = opendataviz_view_check_operation($view->user_id, TRUE);
    if($opration['_edit']) {
      if ($result = _update_opendata_instance_view($id,$title, $input_data, $key)) {
        $data = array(
            'status'   => TRUE,
            'message'  => 'ok',
            'data'     => $view,
        );
      }
    }
  }
  
  print json_encode($data);
  die();
}

function opendataviz_instance_view_save($key) {
  $input_data = $_POST['data'];
  $key = check_plain($_POST['key']);
  $title = check_plain($_POST['title']);
  
  $data = array(
      'status'  => FALSE,
      'message' => 'Invalid operation',
  );
  
  if($opration = opendataviz_view_check_operation($view->user_id, TRUE)) {
    $data['operation'] = $opration;
    $data['user_id'] = $view->user_id;
    if($opration['_create']) {
      $result = _save_opendata_instance_view($title, $input_data, $key);
      
      $data = array(
          'status' => TRUE,
          'message' => 'ok',
          'data' => $result,
          'title' => $title,
          'html'=> ''
      );
    }
  }
  print json_encode($data);
  die();
}

function opendataviz_instance_view_delete($id) {
  $data = array(
      'status'   => FALSE,
      'message'  => 'Not Allowed',
      'view_id'  => $id,
      'title'    => '',
      'html'     => ''
  );
  
  if($result = _delete_opendata_instance_view($id)) {
    $data = array(
        'status'   => TRUE,
        'message'  => 'ok',
        'view_id'  => $id,
        'title'    => '',
        'html'     => ''
    );
  }
  
  print json_encode($data);
  die();
}

function opendataviz_instance_view_title($instance) {
  $input_data = $_POST['data'];
  $key = $_POST['key'];
  
  if ($result = _update_opendata_instance_title($input_data['id'], $input_data['title'])) {
    $data = array(
        'status'   => TRUE,
        'message'  => 'ok',
        'data'     => _get_visualization_instance_view_by_id($input_data['id'])
    );
  } 
  else {
    $data = array(
        'status'  => FALSE,
        'message' => 'Invalid operation',
    );
  }
  
  print json_encode($data);
  die();
}

function _update_opendata_instance_title($id, $title) {
  if ($title = check_plain($title)) {
  //if (drupal_write_record('opendataviz_instance_view', $data, 'id')) {
    if (db_query("UPDATE {opendataviz_instance_view} SET title = '%s' WHERE id = %d", $title, $id)) {
      return TRUE;
    }
  }
  return FALSE;
}


function _save_opendata_instance_view($title, $input_data, $data_instance_key) {
  if ($result = _get_visualization_instance_by_key($data_instance_key)) {
    global $user;
    $input_data_serial = serialize($input_data);
    
    $data = array(
        'data_instance_id' => $result->id,
        'user_id'          => $user->uid,
        'title'            => $title,
        'data'             => $input_data_serial
    );
    
    //if (drupal_write_record('opendataviz_instance_view', $data)) {
    if (db_query("INSERT INTO {opendataviz_instance_view}(data_instance_id, user_id, title, data) VALUES(%d, %d, '%s', '%s') ", $result->id, $user->uid, $title, $input_data_serial)) {
      $data['id'] = mysql_insert_id();
      return $data;
    }
  }
  
  return FALSE;
}

function _update_opendata_instance_view($id, $title, $input_data, $data_instance_key) {
  if ($result = _get_visualization_instance_by_key($data_instance_key)) {
    global $user;
    $input_data_serial = serialize($input_data);
    
    $data = array(
        'id'               => $id,
        'data'             => $input_data_serial
    );
    
    if (db_query("UPDATE {opendataviz_instance_view} SET data= '%s' WHERE id = %d ", $input_data_serial, $id)) {
      return $data;
    }
  }
  
  return FALSE;
}

function _delete_opendata_instance_view($view_id) {
  if ($view = _get_visualization_instance_view_by_id($view_id) ) {
    if ($view->operation['_delete']) {
      if ($result = db_query("DELETE FROM {opendataviz_instance_view} WHERE id = %d", $view_id))
        return TRUE;
    }
  }
  
  return FALSE;
}

function opendataviz_instance_view_default($instance) {
  $input_data = $_POST['data'];
  $data = array(
      'status'  => FALSE,
      'message' => 'Invalid operation',
  );
  $input_data['id'] = check_plain($input_data['id']);
  if($input_data['id'] != '') {
    if (_update_opendata_instance_default($input_data['id'])) {
      $data = array(
          'status'   => TRUE,
          'message'  => 'ok',
          'data'     => _get_visualization_instance_view_by_id($input_data['id'])
      );
    }
  }
  print json_encode($data);
  die();
}

function _update_opendata_instance_default($id) {
  if ($result = _get_visualization_instance_view_by_id($id)) { 
    if (db_query("UPDATE {opendataviz_instance_view} SET `default` = 0 WHERE `data_instance_id` = %d ", $result->data_instance_id)) {
      if (db_query("UPDATE {opendataviz_instance_view} SET `default` = 1 WHERE `id` = %d ", $id)) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

function opendataviz_instance_view_public($instance) {
  $input_data = $_POST['data'];
  $data = array(
      'status'  => FALSE,
      'message' => 'Invalid operation',
  );
  $input_data['id'] = check_plain($input_data['id']);
  if($input_data['id'] != '') {
    if (_opendataviz_instance_view_public($input_data['id'])) {
      $data = array(
          'status'   => TRUE,
          'message'  => 'ok',
          'data'     => _get_visualization_instance_view_by_id($input_data['id'])
      );
    }
  }
  
  print json_encode($data);
  die();
}

function opendataviz_instance_view_private($instance) {
  $input_data = $_POST['data'];
  $data = array(
      'status'  => FALSE,
      'message' => 'Invalid operation',
  );
  $input_data['id'] = check_plain($input_data['id']);
  if($input_data['id'] != '') {
    if (_opendataviz_instance_view_private($input_data['id'])) {
      $data = array(
          'status'   => TRUE,
          'message'  => 'ok',
          'data'     => _get_visualization_instance_view_by_id($input_data['id'])
      );
    }
  }
  print json_encode($data);
  die();
}

function _opendataviz_instance_view_public($id) {
  if ($result = _get_visualization_instance_view_by_id($id)) {
    if (db_query("UPDATE {opendataviz_instance_view} SET `is_public` = 1 WHERE `id` = %d ", $id)) {
      return TRUE;
    }
  }
  return FALSE;
}

function _opendataviz_instance_view_private($id) {
  if ($result = _get_visualization_instance_view_by_id($id)) {
    if (db_query("UPDATE {opendataviz_instance_view} SET `is_public` = 0 WHERE `id` = %d ", $id)) {
      return TRUE;
    }
  }
  return FALSE;
}


