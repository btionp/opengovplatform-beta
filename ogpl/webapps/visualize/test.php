<?php 
$currdir = getcwd();

chdir($_SERVER['DOCUMENT_ROOT']);

require_once("./includes/bootstrap.inc");

drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL); 

global $user;

chdir($currdir);
?>
<p>USER VAR :</p>
<?php 
print_r( $user ); 
