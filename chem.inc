<?php

// $Id$

class IslandoraChem {

  function __construct($pid = '') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {
      $this->pid = $pid;
      $this->item = new Fedora_Item($pid);
    }
  }

  public function showMOL() {

    global $base_url;
    global $base_path;
    global $user;
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');

    $tabset = array();

    $tabset['first_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Description'),
    );

    $tabset['first_tab']['tabs'] = array(
      '#type' => 'tabset',
    );

    //MOL is the datastream id
    $path = drupal_get_path('module', 'Fedora_Repository');
    $fullPath = base_path() . $path;
    $content = "";
    $pathTojs = drupal_get_path('module', 'islandora_chem_sp') . '/js/';

    $molURL = $base_url . base_path() . 'fedora/repository/' . $this->pid . '/MOL/MOL.mol';
    $pathTojs = drupal_get_path('module', 'islandora_chem_sp') . '/js/';
    drupal_add_js("$pathTojs" . 'jmol/Jmol.js');
    $content1 .= '<div id="viewer"></div>';

    $content1 .= '<script>jmolInitialize("/sites/all/modules/islandora_solution_pack_chemistry/js/jmol"); 
                  jmolSetDocument(false);
                  var s2 = jmolApplet(300, "load ' . $molURL . '");
                  document.getElementById("viewer").innerHTML=s2;
                  </script>';

    drupal_add_js("$pathTojs" . 'jsv/JSVfunctions.js', 'inline', 'header');
    drupal_add_js('jsvls="load ' . $base_url . '/fedora/repository/' . $this->pid . '/IR/IR.jdx; reverseplot true; gridon false; plotcolor 150,100,200";
                  insertJSVObject("' . $base_url . '/' . $pathTojs . 'jsv/jspecview.jar","JSVApplet","600","400",jsvls);
                  ', 'inline', 'footer');

    $content2 = '<img src="' . $base_url . '/fedora/repository/' . $this->pid . '/MEDIUM/MEDIUM.jpg" />';
    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "CML");

    if (!$xmlstr == NULL) {
      $xml = new SimpleXMLElement($xmlstr);
      $title = $xml->molecule->attributes()->title;
//      $author = $xml->xpath('/chemical:chemical/chemical:author');
//      $date = $xml->xpath('/chemical:chemical/chemical:date');
      $mw = $xml->molecule->property->scalar;
      $formula = $xml->molecule->formula->attributes()->concise;
//      $author2 = $author[0][0];
//      $date2 = $date[0][0];
      $mw2 = $mw[0][0];
      $formula2 = str_replace(' ', '', $formula);
      $content2 .= '<div id="title" class="clearfix"> Title: ' . $title . '</div>';
//      $content2 .= '<div id="author">Author: ' . $author2 . '</div>';
//      $content2 .= '<div id="date">Date: ' . $date2 . '</div>';
      $content2 .= '<div id="date">MW: ' . $mw2 . '</div>';
      $content2 .= '<div id="date">Formula: ' . $formula2 . '</div>';
    }
    else {
      $content2 .= '<div id="title">No XML datastream found</div>';
    }
    $smi = $collectionHelper->getStream($this->pid, "SMI");

    if (!$smi == NULL) {
      $content2 .= '<div id="smiles">SMILES: ' . $smi . '</div>';
    }
    else {
      $content2 .= '<div id="smiles">No SMILES datastream found</div>';
    }

    $content3 = "1H NMR";

    $content4 = "13C NMR";

    $content5 = "IR";
    $content5 .= '<div id="jsvls"></div>';

    $tabset['first_tab']['tabs']['summary'] = array(
      '#type' => 'tabpage',
      '#title' => t('Summary'),
      '#content' => $content2,
    );

    $tabset['first_tab']['tabs']['view'] = array(
      '#type' => 'tabpage',
      '#title' => t('3D view'),
      '#content' => $content1,
    );

//    $tabset['first_tab']['tabs']['1hnmr'] = array(
//      '#type' => 'tabpage',
//      '#title' => t('1H NMR'),
//      '#content' => $content3,
//    );
//
//    $tabset['first_tab']['tabs']['13cnmr'] = array(
//      '#type' => 'tabpage',
//      '#title' => t('13C NMR'),
//      '#content' => $content4,
//    );
//
//    $tabset['first_tab']['tabs']['ir'] = array(
//      '#type' => 'tabpage',
//      '#title' => t('IR'),
//      '#content' => $content5,
//    );

    return $tabset;
  }

  function chem2mol($parameterArray = NULL, $dsid, $file, $file_ext) {
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);

    if (!file_exists($file . "svg")) {
      system("obabel \"$file\" --title -omol -xw -O\"$filename\" > /dev/null 2>&1", $returnValue);
    }
    else {
      $returnValue = '0';
    }

    $returnValue = '0';

    if ($returnValue == '0') {
      $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
      return TRUE;
    }
    else {
      return $returnValue;
    }
  }

  function chem2jpg($parameterArray, $dsid, $file, $file_ext) {
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
    $height = $parameterArray['height'];
    $width = $parameterArray['width'];
    $returnValue = TRUE;

    drupal_set_message('Filename is: ' . $filename);

    if (!file_exists($file . "svg")) {
      system("obabel \"$file\" --title -osvg -xw -O\"$file\".svg &>/dev/null", $returnvalue);
    }
    else {
      $returnValue = '0';
    }
    $returnValue = '0';

    if (!file_exists($filename)) {
      system("convert -resize \"$width\" -gravity center -extent \"$width\"x\"$width\" \"$file\".svg \"$filename\" 2>&1 &", $returnValue);
    }
    else {
      $returnValue = '0';
    }

    if ($returnValue == '0') {
      $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
      return TRUE;
      print "Ingest succeeded";
    }
    else {
      return $returnValue;
      print "Ingest failed";
    }
  }

  function mol2mw($parameterArray = NULL, $dsid, $file, $file_ext) {
    $returnValue = TRUE;

    exec("obprop \"$file\"", $return);

    $strMW = explode("s", $return[3]);
    echo ltrim($strMW[2]);
    echo "\n";

    if ($returnValue == '0') {
      $_SESSION['fedora_ingest_files']["$dsid"] = $file . $file_suffix;
      return TRUE;
    }
    else {
      return $returnValue;
    }
  }

  function mol2smi($parameterArray = NULL, $dsid, $file, $file_ext) {
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
    if (!file_exists($file . "xml")) {
      system("obabel \"$file\" -o smi -xn -O\"$filename\" > /dev/null 2>&1", $returnValue);
    }
    else {
      $returnValue = '0';
    }
    $returnValue = '0';

    if ($returnValue == '0') {
      $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
      return TRUE;
    }
    else {
      return $returnValue;
    }
  }

  function chem2cml($parameterArray = NULL, $dsid, $file, $file_ext) {
    $file_suffix = '_' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);

    if (!file_exists($filename)) {
      $fh = fopen($filename, 'w');
      exec("obprop \"$file\"", $return);

      $name = $return[0];
      $name = preg_replace('!\s+!', ' ', $name);
      $name = str_replace('"', '', $name);
      $name = ltrim($name, 'name ');
      $mf = $return[1];
      $mf = preg_replace('!\s+!', ' ', $mf);
      $mf_array = explode(' ', $mf);
      $mf_value = $mf_array[1];
      $mf_value = preg_replace('/([A-Z]$)/', '$1 1', $mf_value);
      $mf_value = preg_replace('/([A-Z][a-z]?)/', '$1 ', $mf_value);
      $mf_value = preg_replace('/([0-9]*)/', '$1 ', $mf_value);
      $mf_value = preg_replace('/([A-Z])([A-Z])/', '$1 1 $2', $mf_value);
      $mf_value = preg_replace('!\s+!', ' ', $mf_value);
      $mf_value = trim($mf_value);
      $mw = $return[2];
      $mw = preg_replace('!\s+!', ' ', $mw);
      $mw_array = explode(' ', $mw);
      $mw_value = $mw_array[1];
      $inchi = $return[6];
      $inchi = preg_replace('!\s+!', ' ', $inchi);
      $inchi_array = explode(' ', $inchi);
      $inchi_value = $inchi_array[1];

      $cml = '<cml xmlns="http://www.xml-cml.org/schema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" title="name example" xsi:schemaLocation="http://www.xml-cml.org/schema ../../schema.xsd">' . "\n";
      $cml .= '<molecule title="' . $name . '">' . "\n";
      $cml .= '<property id="p1" title="Molecular weight">' . "\n";
      $cml .= '<scalar id="s1" dictRef="nonSi:mw">';
      $cml .= $mw_value;
      $cml .= '</scalar>' . "\n";
      $cml .= '</property>' . "\n";
      $cml .= '<formula id="f1" concise="' . $mf_value . '"/>' . "\n";
      $cml .= '<identifier xmlns:inchi="http://www.iupac.org/inchi" xmlns="http://www.xml-cml.org/schema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="0.93Beta" tautomeric="0" xsi:schemaLocation="http://www.xml-cml.org/schema ../../schema.xsd">' . "\n";
      $cml .= '<inchi:basic>';
      $cml .= $inchi_value;
      $cml .= '</inchi:basic>' . "\n";
      $cml .= '</identifier>' . "\n";
      $cml .= '</molecule>' . "\n";
      $cml .= '</cml>';
      var_dump($cml);
      fwrite($fh, $cml);
      fclose($fh);
      drupal_set_message('CML file created!');
    }
    else {
      $returnValue = '0';
    }
    $returnValue = '0';

    if ($returnValue == '0') {
      $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
      return TRUE;
    }
    else {
      return $returnValue;
    }
  }
}