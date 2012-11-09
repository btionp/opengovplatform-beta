<?php 
  $validator_flag = false;
  
  if($validator_wf = dms_customizations_get_dataset_validator_details($node->nid)) {
    $validator_flag = true;
    if($_GET['debug'] == 'validator') {
      print '<pre>' . print_r($validator_wf, true) . '</pre>';
    }
  } 
  
?>
<div id="node-<?php print $node->nid; ?>" class="node <?php print $node_classes; ?>">
  <div class="inner">
    <?php print $picture ?>

    <?php if ($page == 0): ?>
    <h2 class="title"><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
    <?php endif; ?>

    <?php if ($submitted): ?>
    <div class="meta">
      <span class="submitted"><?php print $submitted ?></span>
    </div>
    <?php endif; ?>

    <?php if ($node_top && !$teaser): ?>
    <div id="node-top" class="node-top row nested">
      <div id="node-top-inner" class="node-top-inner inner">
        <?php print $node_top; ?>
      </div><!-- /node-top-inner -->
    </div><!-- /node-top -->
    <?php endif; ?>
    <div class="content clearfix">
     <?php /* print $content; */ ?>
     <?php 
  			$args = array($node->nid);
  			function _views_embed_view($name, $display_id = 'default', $args) {
  			  $view = views_get_view($name);
  			  return $view->preview($display_id, $args);
  			}
  		?>
      <div class="pane-dataset basic-info">
  			<h2>Basic Info</h2><?php 
  			    print _views_embed_view('dataset', 'block_1', $args);
  			?></div>
  			<div class="pane-dataset more-info">
  			<h2>More Info</h2><?php 
  			  print _views_embed_view('dataset', 'block_2', $args);
  			?></div>
  			<div class="pane-dataset contact-info">
  			<h2>Contact</h2><?php 
  			  $uid_args = array($validator_wf->uid);
  			  print _views_embed_view('dataset', 'block_3', $uid_args);
  			?></div>
     
    </div>
      <!-- CANCEL BUTTON FOR PREVIEW -->
      <div class="cancel-button-div">
        <?php if (isset($_REQUEST['destination'])): ?>
          <?php  print l(t('Cancel'), '', array('attributes' => array('class' => 'cancel-button'))); ?>
        <?php endif; ?>
      </div>
      <!-- END CANCEL BUTTON -->
    <?php if ($terms): ?>
    <div class="terms">
      <?php print $terms; ?>
    </div>
    <?php endif;?>

    <?php if ($links): ?>
    <div class="links">
      <?php print $links; ?>
      
    </div>
    <?php endif; ?>
  </div><!-- /inner -->

  <?php if ($node_bottom && !$teaser): ?>
  <div id="node-bottom" class="node-bottom row nested">
    <div id="node-bottom-inner" class="node-bottom-inner inner">
      <?php print $node_bottom; ?>
    </div><!-- /node-bottom-inner -->
  </div><!-- /node-bottom -->
  <?php endif; ?>
</div><!-- /node-<?php print $node->nid; ?> -->