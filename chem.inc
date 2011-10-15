<?php

/**
 * @file
 *
 * Provides the functions to process and view chemical structure files.
 */
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

//    $molURL = $base_url . base_path() . 'fedora/repository/' . $this->pid . '/MOL/MOL.mol';
//    $pathTojs = drupal_get_path('module', 'islandora_chem_sp') . '/js/';
//    drupal_add_js("$pathTojs" . 'jmol/Jmol.js');
//    $content1 .= '<div id="viewer"></div>';
//
//    $content1 .= '<script>jmolInitialize("/sites/all/modules/islandora_solution_pack_chemistry/js/jmol"); 
//                  jmolSetDocument(false);
//                  var s2 = jmolApplet(300, "load ' . $molURL . '");
//                  document.getElementById("viewer").innerHTML=s2;
//                  </script>';
//
//    drupal_add_js("$pathTojs" . 'jsv/JSVfunctions.js', 'inline', 'header');
//    drupal_add_js('jsvls="load ' . $base_url . '/fedora/repository/' . $this->pid . '/IR/IR.jdx; reverseplot true; gridon false; plotcolor 150,100,200";
//                  insertJSVObject("' . $base_url . '/' . $pathTojs . 'jsv/jspecview.jar","JSVApplet","600","400",jsvls);
//                  ', 'inline', 'footer');

    $content2 = '<img src="' . $base_url . '/fedora/repository/' . $this->pid . '/MEDIUM/MEDIUM.jpg" />';
    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "CML");
    $collectionHelper2 = new CollectionClass();
    $xmlstr2 = $collectionHelper2->getStream($this->pid, "DC");

    if (!$xmlstr == NULL) {
      $xml2 = new SimpleXMLElement($xmlstr2);
      $title = $xml2->children('dc', TRUE);
      $xml = new SimpleXMLElement($xmlstr);
      $desc = $xml->molecule->attributes()->title;
//      $author = $xml->xpath('/chemical:chemical/chemical:author');
//      $date = $xml->xpath('/chemical:chemical/chemical:date');
      $mw = $xml->molecule->property->scalar;
      $formula = $xml->molecule->formula->attributes()->concise;
      $inchi = $xml->molecule->identifier->children('inchi', TRUE)->basic;
      $inchi_key = $xml->molecule->identifier->children('inchi', TRUE)->key;
//      $author2 = $author[0][0];
//      $date2 = $date[0][0];
      $mw2 = $mw[0][0];
      $formula2 = str_replace(' ', '', $formula);
      $formula2 = preg_replace('/([0-9]*)/', '<sub>$1</sub>', $formula2);

      $url = 'http://cactus.nci.nih.gov/chemical/structure/' . urlencode($inchi_key) . '/names/xml';

      $iupac_xml = new DOMDocument();
      $iupac_xml->load($url);
      $iupac = $iupac_xml->getElementsByTagName('item')->item(0)->nodeValue;
      $iupac = ucfirst(strtolower($iupac));
      if ($iupac == NULL) {
        $iupac = 'Not found';
      };

      $synonym = $xml->molecule->alternative;
      $synonymno = $synonym->count();

      switch ($synonymno) {
        case 0:
          $synonyms = 'None found';
          break;
        case 1:
          $synonyms = ucfirst(strtolower($synonym[0]));
          break;
        case 2:
          $synonyms = ucfirst(strtolower($synonym[0])) . ', ' . ucfirst(strtolower($synonym[1]));
          break;
        default:
          $synonyms = ucfirst(strtolower($synonym[0])) . ', ' . ucfirst(strtolower($synonym[1])) . ', ' . ucfirst(strtolower($synonym[2]));
      }

      if ($xml->molecule->metadata) {
        $spider_url = $xml->molecule->metadata->attributes()->content;
        $spider_entry = l($spider_url, $spider_url);
      }
      else {
        $spider_entry = 'Not found';
      }

      $header = array();
      $rows = array(
        array('Title', $title),
        array('IUPAC name', $iupac),
        array('Synonyms', html_entity_decode($synonyms)),
        array('File description', $desc),
        array('Molecular weight', $mw2),
        array('Molecular formula', $formula2),
        array('Inchi', $inchi),
        array('Inchikey', $inchi_key),
        array('URL', $spider_entry),
      );

      $content2 .= theme('table', $header, $rows);
    }
    else {
      $content2 .= '<div id="title">No XML datastream found</div>';
    }

    drupal_add_css($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb.css');
    drupal_add_js($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb-libs.js');
    drupal_add_js($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb.js');

    $file = trim(file_get_contents($base_url . base_path() . 'fedora/repository/' . $this->pid . '/MOL'));

    $file = str_replace("\n", '\n', $file);
    $file = str_replace("\r", '', $file);
    $file = 'Molecule Name\n' . $file;

    $content1 = '<script>
                    var transformBallAndStick = new ChemDoodle.TransformCanvas3D("transformBallAndStick", 250, 250);
                    transformBallAndStick.specs.set3DRepresentation("Ball and Stick");
                    transformBallAndStick.specs.backgroundColor = "black";
                    var molFile = "' . $file . '";
                    var molecule = ChemDoodle.readMOL(molFile, 1);
                    transformBallAndStick.loadMolecule(molecule);
                 </script>';

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

    $tabset['first_tab']['tabs']['1hnmr'] = array(
      '#type' => 'tabpage',
      '#title' => t('1H NMR'),
      '#content' => $content6,
    );
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
      system("obabel \"$file\" --title -omol -xw -O\"$filename\" --gen3D > /dev/null 2>&1", $returnValue);
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

    if (!file_exists($file . "svg")) {
      system("obabel \"$file\" --title -osvg -xw -O\"$file\".svg &>/dev/null", $returnvalue);
    }
    else {
      $returnValue = '0';
    }
    $returnValue = '0';

    if (!file_exists($filename)) {
      system("convert -resize \"$width\" -gravity center -extent \"$width\"x\"$width\" \"$file\".svg \"$filename\" 2>&1 &", $returnValue);
      unlink($file . '.svg');
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

  function chem2rdf($parameterArray = NULL, $dsid, $file, $file_ext) {
    $file_suffix = '_' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);

    exec("obprop \"$file\"", $return);
    $inchi = $return[6];
    $inchi = preg_replace('!\s+!', ' ', $inchi);
    $inchi_array = explode(' ', $inchi);
    $inchi_value = $inchi_array[1];

    $url = 'http://rdf.chemspider.com/' . $inchi_value;

    if (!file_exists($filename)) {
      $xml = new DOMDocument();
      $xml->load($url);
      $xml->save($filename);
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
      exec("obabel \"$file\" -oinchi -xK", $inchi_key);

      $name = $return[0];
      $name = preg_replace('!\s+!', ' ', $name);
      $name = str_replace('"', '', $name);
      $name = ltrim($name, 'name ');
      $mf = $return[1];
      $mf = preg_replace('!\s+!', ' ', $mf);
      $mf_array = explode(' ', $mf);
      $mf_value = $mf_array[1];
      $mf_value = preg_replace('/([A-Z][a-z]?)/', '$1 ', $mf_value);
      $mf_value = preg_replace('/([0-9]*)/', '$1 ', $mf_value);
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
      $rdf = new DOMDocument();
      $rdf_url = 'http://rdf.chemspider.com/' . $inchi_key[0];

      drupal_set_message(implode(', ', get_headers($rdf_url)));
      $url_headers = get_headers($rdf_url);
      
      if ($url_headers[0] == 'HTTP/1.1 302 Found') {
        $rdf->load($rdf_url);
        $trivial = $rdf->getElementsByTagName('Description')->item(13)->nodeValue;

        if ($rdf->getElementsByTagName('Description')->item(1)) {
          $spider_url = $rdf->getElementsByTagName('Description')->item(1)->getAttribute('rdf:about');
        }
        else {
          $spider_url = '';
        }
      }
      else {
        $trivial = 'Not found';
        $spider_url = 'Not found';
      }

      $cml = '<cml xmlns="http://www.xml-cml.org/schema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" title="name example" xsi:schemaLocation="http://www.xml-cml.org/schema ../../schema.xsd">' . "\n";
      $cml .= '<molecule title="' . $name . '">' . "\n";
      $cml .= '<name id="n0" convention="IUPAC:trivial">';
      $cml .= trim($trivial);
      $cml .= '</name>' . "\n";

      $url = 'http://cactus.nci.nih.gov/chemical/structure/' . urlencode($inchi_key[0]) . '/names/xml';

      $iupac_xml = new DOMDocument();
      $iupac_xml->load($url);
      $nodes = $iupac_xml->getElementsByTagName('item');
      $nodeno = $nodes->length;
      for ($i = 0; $i < $nodeno; $i++) {
        $iupac = $iupac_xml->getElementsByTagName('item')->item($i)->nodeValue;
        $iupac = ucfirst(strtolower($iupac));
        $cml .= '<alternative id="a' . ($i + 1) . '" type="synonym">' . htmlentities(trim($iupac)) . '</alternative>' . "\n";
      }

      $cml .= '<property id="p1" title="Molecular weight">' . "\n";
      $cml .= '<scalar id="s1" dictRef="nonSi:mw">';
      $cml .= $mw_value;
      $cml .= '</scalar>' . "\n";
      $cml .= '</property>' . "\n";
      $cml .= '<formula id="f1" concise="' . $mf_value . '" />' . "\n";
      $cml .= '<identifier xmlns:inchi="http://www.iupac.org/inchi" xmlns="http://www.xml-cml.org/schema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="0.93Beta" tautomeric="0" xsi:schemaLocation="http://www.xml-cml.org/schema ../../schema.xsd">' . "\n";
      $cml .= '<inchi:basic>';
      $cml .= $inchi_value;
      $cml .= '</inchi:basic>' . "\n";
      $cml .= '<inchi:key>';
      $cml .= $inchi_key[0];
      $cml .= '</inchi:key>' . "\n";
      $cml .= '</identifier>' . "\n";
      $cml .= '<metadata id="m1" name="dc:identifier" content="' . $spider_url . '" />' . "\n";
      $cml .= '</molecule>' . "\n";
      $cml .= '</cml>';
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