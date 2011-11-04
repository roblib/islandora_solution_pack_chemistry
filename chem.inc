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
/**
 * Build the form to show the relevant datastreams for this object.
 * @global type $base_url
 * @global type $base_path
 * @global type $user
 * @return type $tabset 
 */
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
//    Create path variables 
    $path = drupal_get_path('module', 'Fedora_Repository');
    $fullPath = base_path() . $path;
    $content = "";
    $pathTojs = drupal_get_path('module', 'islandora_chem_sp') . '/js/';
//    Show the medium sized image of the structure at the top of the view
    $content1 = '<img src="' . $base_url . '/fedora/repository/' . $this->pid . '/MEDIUM/MEDIUM.jpg" />';
//    Load the XML from the CML and DC datastreams
    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "CML");
    $collectionHelper2 = new CollectionClass();
    $xmlstr2 = $collectionHelper2->getStream($this->pid, "DC");
//    If the CML datastream exists then display the relevant information.
//    It's unlikely that the DC stream will exist if the CML one doesn't
    if (!$xmlstr == NULL) {
//      Pull out the elements from the XML
      $xml2 = new SimpleXMLElement($xmlstr2);
      $title = $xml2->children('dc', TRUE);
      $xml = new SimpleXMLElement($xmlstr);
      $ns_xml = $xml->children();
      $desc = $ns_xml->molecule->attributes()->title;
      $mw = $ns_xml->xpath('//scalar');
      $formula = $ns_xml->molecule->formula->attributes()->concise;
      $inchi = $ns_xml->molecule->identifier->inchi;
      $inchi_key = $ns_xml->molecule->identifier->inchikey;
      $mw2 = $mw[0][0];
//      The formula is stored with spaces in to conform to the CML standard so we need to get rid of them
//      Also make the numbers subscript 
      $formula2 = str_replace(' ', '', $formula);
      $formula2 = preg_replace('/([0-9]*)/', '<sub>$1</sub>', $formula2);
//      Clean up IUPAC name as it's usually ugly. If there isn't one then deal with it gracefully
      $iupac = $ns_xml->molecule->name;
      $iupac = ucfirst(strtolower($iupac));
      if ($iupac == NULL) {
        $iupac = 'Not found';
      };

      $synonym = $ns_xml->xpath("//alternative[@id='s1']");
      $fragments = $ns_xml->xpath("//alternative[@id='f1']");
//      Get the URL to chemspider if it exists and create a link from it
      $spider_url = $ns_xml->molecule->metadata->attributes()->content;

      if ($spider_url != '') {
        $spider_entry = l($spider_url, $spider_url);
      }
      else {
        $spider_entry = 'Not found';
      }
//      Create a table to display all the data nicely
      $header = array();
      $rows = array(
        array('Title', $title),
        array('IUPAC name', $iupac),
        array('Synonyms', html_entity_decode($synonym[0])),
        array('File description', $desc),
        array('Molecular weight', $mw2),
        array('Molecular formula', $formula2),
        array('Inchi', $inchi),
        array('Inchikey', $inchi_key),
        array('Fragments', $fragments[0]),
        array('URL', $spider_entry),
      );

      $content1 .= theme('table', $header, $rows);
    }
    else {
      $content1 .= '<div id="title">No XML datastream found</div>';
    }

    drupal_add_css($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb.css');
    drupal_add_js($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb-libs.js');
    drupal_add_js($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb.js');
//    Grab the structure file and clean it up so that it displays in the JS viewer
    $file = trim(file_get_contents($base_url . base_path() . 'fedora/repository/' . $this->pid . '/MOL'));

    $file = str_replace("\n", '\n', $file);
    $file = str_replace("\r", '', $file);
    $file = 'Molecule Name\n' . $file;
//    Put in the JS for the viewer and load the MOL file
    $content2 = '<script>
                    var transformBallAndStick = new ChemDoodle.TransformCanvas3D("transformBallAndStick", 250, 250);
                    transformBallAndStick.specs.set3DRepresentation("Ball and Stick");
                    transformBallAndStick.specs.backgroundColor = "black";
                    var molFile = "' . $file . '";
                    var molecule = ChemDoodle.readMOL(molFile, 1);
                    transformBallAndStick.loadMolecule(molecule);
                 </script>';


    $tabset['first_tab']['tabs']['summary'] = array(
      '#type' => 'tabpage',
      '#title' => t('Summary'),
      '#content' => $content1,
      '#weight' => 1,
    );

    $tabset['first_tab']['tabs']['view'] = array(
      '#type' => 'tabpage',
      '#title' => t('3D view'),
      '#content' => $content2,
      '#weight' => 2,
    );
//    Display the raw output file as text if it exists
    $collectionHelper_output = new CollectionClass();
    $xmlstr_output = $collectionHelper_output->getStream($this->pid, "OBJ");
    
    if (!$xmlstr_output == NULL) {
      $content3 = str_replace("\n", '</BR>', file_get_contents($base_url . base_path() . 'fedora/repository/' . $this->pid . '/OBJ'));
    }
    else {
      $content3 = '<div id="title">No OBJ datastream found</div>';
    }
    
    $tabset['first_tab']['tabs']['output'] = array(
      '#type' => 'tabpage',
      '#title' => t('Output file'),
      '#content' => $content3,
      '#weight' => 3,
    );
//    Display the raw input file as text if it exists
    $collectionHelper_input = new CollectionClass();
    $xmlstr_input = $collectionHelper_input->getStream($this->pid, "INPUT");
    
    if (!$xmlstr_input == NULL) {
      $content4 = str_replace("\n", '</BR>', file_get_contents($base_url . base_path() . 'fedora/repository/' . $this->pid . '/INPUT'));
      
      $tabset['first_tab']['tabs']['input'] = array(
      '#type' => 'tabpage',
      '#title' => t('Input file'),
      '#content' => $content4,
      '#weight' => 4,
    );
    }
//    Display the raw code as text if it exists
    $collectionHelper_code = new CollectionClass();
    $xmlstr_code = $collectionHelper_code->getStream($this->pid, "CODE");
    
    if (!$xmlstr_code == NULL) {
      $content5 = str_replace("\n", '</BR>', file_get_contents($base_url . base_path() . 'fedora/repository/' . $this->pid . '/CODE'));
      
      $tabset['first_tab']['tabs']['code'] = array(
      '#type' => 'tabpage',
      '#title' => t('Code'),
      '#content' => $content5,
      '#weight' => 5,
    );
    }
    
    return $tabset;
  }

  /**
   * Function to create a standard MOL file from whatever format is ingested
   * @param type $parameterArray
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return string 
   */
  function chem2mol($parameterArray = NULL, $dsid, $file, $file_ext) {
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
//  Call openbabel and convert the file to MOL. Create the 3D structure so that it looks good in the 3D viewer
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

  /**
   * Function to create a jpg from the structure file
   * @param type $parameterArray
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return string 
   */
  function chem2jpg($parameterArray, $dsid, $file, $file_ext) {
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
    $height = $parameterArray['height'];
    $width = $parameterArray['width'];
    $returnValue = TRUE;
//  OpenBabel won't convert directly to jpg so we need to go via svg
    if (!file_exists($file . "svg")) {
      system("obabel \"$file\" --title -osvg -xw -d -O\"$file\".svg &>/dev/null", $returnvalue);
    }
    else {
      $returnValue = '0';
    }
    $returnValue = '0';
//  Once the svg is created we need to convert it to jpg using imagemagick
    if (!file_exists($filename)) {
      system("convert \"$file\".svg -resize \"$width\"x\"$width\" -gravity center -extent \"$width\"x\"$width\" \"$filename\" 2>&1 &", $returnValue);
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

  /**
   * Function to generate data from structure file and create an XML file
   * @param type $parameterArray
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return string 
   */
  public function chem2cml($parameterArray = NULL, $dsid, $file, $file_ext) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $file_suffix = '_' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
//  Run various system calls to generate the data
    if (!file_exists($filename)) {
      exec("obprop \"$file\"", $return, $returnValue1);
      if ($returnValue1 != '1') {
        drupal_set_message(t('obprop failed to run properly'), 'error');
        return $returnValue1;
        }
      exec("obabel \"$file\" -oinchi -xK", $inchi_key, $returnValue2);
      if ($returnValue2 != '0') {
        drupal_set_message(t('The InChIKey could not be calculated'), 'error');
        return $returnValue2;
        }
      exec("obabel \"$file\" -omol -O/tmp/checkmol.mol", $checkreturn, $returnValue3);
      if ($returnValue3 != '0') {
        drupal_set_message(t('The MOL file could not be generated'), 'error');
        return $returnValue3;
        }
      exec("checkmol -c /tmp/checkmol.mol", $fragment, $returnValue4);
      if ($returnValue4 != '0') {
        drupal_set_message(t('checkmol failed to run properly'), 'error');
        return $returnValue4;
      }
//      This calls a python script to generate the molecular fingerprint
      exec('/usr/bin/python ' . drupal_get_path('module', 'islandora_chem_sp') . '/indigo-python-api-1.0.0-linux/fingerprint.py /tmp/checkmol.mol', $fingerprint, $returnValue5);
      if ($returnValue5 != '0') {
        drupal_set_message(t('The molecular fingerprint could not be calculated'), 'error');
        return $returnValue5;
        }
//      The python script leaves some data in the string it generates so this is stripped out
      $fingerprint = explode(', ', str_replace(array('[', ']', 'L'), array('', '', ''), $fingerprint[0]));
//      The returned arrays are parsed to obtain the relevant data
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
//      The chemspider API is queried to retrieve the IUPAC name of the compound
      $rdf = new DOMDocument();
      $rdf_url = 'http://rdf.chemspider.com/' . htmlentities($inchi_key[0]);

      $url_headers = get_headers($rdf_url);

      if ($url_headers[0] == 'HTTP/1.1 302 Found') {
        $rdf->load($rdf_url);
        $trivial = $rdf->getElementsByTagName('Description')->item(13)->nodeValue;
//    The URL to the chemspider entry is also generated
        if ($rdf->getElementsByTagName('Description')->item(1)) {
          $spider_url = $rdf->getElementsByTagName('Description')->item(1)->getAttribute('rdf:about');
        }
        else {
          $spider_url = '';
        }
      }
      else {
        $trivial = 'Not found';
        $spider_url = '';
      }

      $fragment = trim(str_replace(';', ' ', $fragment[0]));
//    The molecular fingerprint consists of 117 values which are put into a MySQL db
//    This allows for substructure searching using a bitwise comparison
      for ($i = 0; $i < 117; $i++) {
        ${fpv2 . $i} = $fingerprint[$i];
      }
//    A check is done initially to see if this molecule has been stored before
      $query2 = db_query("SELECT id FROM {islandora_chem_fingerprints} WHERE inchikey = '%s'", $inchi_key[0]);

      $test2 = NULL;
      
      while ($obj2 = db_fetch_object($query2)) {
        $test2 = $obj2->id;
      }

      if (!isset($test2)) {
        $result = db_query("INSERT INTO {islandora_chem_fingerprints} (inchikey, fp0, fp1, fp2, fp3, fp4, fp5, fp6, fp7, fp8, fp9, fp10, fp11, fp12, fp13, fp14, fp15, fp16, fp17, fp18, fp19, fp20, fp21, fp22, fp23, fp24, fp25, fp26, fp27, fp28, fp29, fp30, fp31, fp32, fp33, fp34, fp35, fp36, fp37, fp38, fp39, fp40, fp41, fp42, fp43, fp44, fp45, fp46, fp47, fp48, fp49, fp50, fp51, fp52, fp53, fp54, fp55, fp56, fp57, fp58, fp59, fp60, fp61, fp62, fp63, fp64, fp65, fp66, fp67, fp68, fp69, fp70, fp71, fp72, fp73, fp74, fp75, fp76, fp77, fp78, fp79, fp80, fp81, fp82, fp83, fp84, fp85, fp86, fp87, fp88, fp89, fp90, fp91, fp92, fp93, fp94, fp95, fp96, fp97, fp98, fp99, fp100, fp101, fp102, fp103, fp104, fp105, fp106, fp107, fp108, fp109, fp110, fp111, fp112, fp113, fp114, fp115, fp116) VALUES ('$inchi_key[0]', $fpv20, $fpv21, $fpv22, $fpv23, $fpv24, $fpv25, $fpv26, $fpv27, $fpv28, $fpv29, $fpv210, $fpv211, $fpv212, $fpv213, $fpv214, $fpv215, $fpv216, $fpv217, $fpv218, $fpv219, $fpv220, $fpv221, $fpv222, $fpv223, $fpv224, $fpv225, $fpv226, $fpv227, $fpv228, $fpv229, $fpv230, $fpv231, $fpv232, $fpv233, $fpv234, $fpv235, $fpv236, $fpv237, $fpv238, $fpv239, $fpv240, $fpv241, $fpv242, $fpv243, $fpv244, $fpv245, $fpv246, $fpv247, $fpv248, $fpv249, $fpv250, $fpv251, $fpv252, $fpv253, $fpv254, $fpv255, $fpv256, $fpv257, $fpv258, $fpv259, $fpv260, $fpv261, $fpv262, $fpv263, $fpv264, $fpv265, $fpv266, $fpv267, $fpv268, $fpv269, $fpv270, $fpv271, $fpv272, $fpv273, $fpv274, $fpv275, $fpv276, $fpv277, $fpv278, $fpv279, $fpv280, $fpv281, $fpv282, $fpv283, $fpv284, $fpv285, $fpv286, $fpv287, $fpv288, $fpv289, $fpv290, $fpv291, $fpv292, $fpv293, $fpv294, $fpv295, $fpv296, $fpv297, $fpv298, $fpv299, $fpv2100, $fpv2101, $fpv2102, $fpv2103, $fpv2104, $fpv2105, $fpv2106, $fpv2107, $fpv2108, $fpv2109, $fpv2110, $fpv2111, $fpv2112, $fpv2113, $fpv2114, $fpv2115, $fpv2116)");
        if ($result == FALSE) {
          drupal_set_message(t('Database update failed!', 'warning'));
        }
      }
//      The XML is created and populated with the data obtained above
      $dom = new DOMDocument();

      $cml = $dom->createElementNS("http://www.xml-cml.org/schema", "cml:cml");
      $dom->appendChild($cml);
      
      $xmlname = $dom->createElement("molecule");
      $xmlname->setAttribute("title", $name);
      $cml->appendChild($xmlname);

      $xmltrivial = $dom->createElement("name", $trivial);
      $xmltrivial->setAttribute("id", "n0");
      $xmltrivial->setAttribute("convention", "IUPAC:trivial");
      $xmlname->appendChild($xmltrivial);
//    Synonyms are retrieved from the pubchem/NCI db and added to the XML for searching
      $url = 'http://cactus.nci.nih.gov/chemical/structure/' . urlencode($inchi_key[0]) . '/names/xml';

      $iupac_xml = new DOMDocument();
      $iupac_xml->load($url);
      $nodes = $iupac_xml->getElementsByTagName('item');
      $nodeno = $nodes->length;

      $synonym = '';

      for ($i = 0; $i < $nodeno; $i++) {
        $iupac = $iupac_xml->getElementsByTagName('item')->item($i)->nodeValue;
        $iupac = htmlentities(ucfirst(strtolower($iupac)));
        $synonym .= ' ' . $iupac;
      }

      $xmlsynonym = $dom->createElement("alternative", $synonym);
      $xmlsynonym->setAttribute("id", "s1");
      $xmlsynonym->setAttribute("type", "synonym");
      $xmlname->appendChild($xmlsynonym);

      $xmlfragment = $dom->createElement("alternative", $fragment);
      $xmlfragment->setAttribute("id", "f1");
      $xmlfragment->setAttribute("type", "fragment");
      $xmlname->appendChild($xmlfragment);

      $xmlproperty = $dom->createElement("property");
      $xmlproperty->setAttribute("id", "p1");
      $xmlproperty->setAttribute("title", "Molecular Weight");
      $xmlname->appendChild($xmlproperty);

      $xmlscalar = $dom->createElement("scalar", $mw_value);
      $xmlscalar->setAttribute("id", "s1");
      $xmlscalar->setAttribute("dictRef", "nonSi:mw");
      $xmlproperty->appendChild($xmlscalar);

      $xmlformula = $dom->createElement("formula");
      $xmlformula->setAttribute("id", "f1");
      $xmlformula->setAttribute("concise", $mf_value);
      $xmlname->appendChild($xmlformula);

      $xmlidentifier = $dom->createElement("identifier");
      $xmlname->appendChild($xmlidentifier);

      $xmlinchi = $dom->createElement("inchi", $inchi_value);
      $xmlidentifier->appendChild($xmlinchi);

      $xmlinchikey = $dom->createElement("inchikey", $inchi_key[0]);
      $xmlidentifier->appendChild($xmlinchikey);

      $xmlmetadata = $dom->createElement("metadata");
      $xmlmetadata->setAttribute("id", "m1");
      $xmlmetadata->setAttribute("name", "dc:identifier");
      $xmlmetadata->setAttribute("content", $spider_url);
      $xmlname->appendChild($xmlmetadata);

      $dom->save($filename);
    }
    else {
      drupal_set_message(t('XML file already exists in the filesytem!'), 'error');
      $returnValue = FALSE;
    }

    if ($returnValue == TRUE) {      
      $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
      $_SESSION['fedora_ingest_method']["$dsid"] = 'X';
      return TRUE;
    }
    else {
      return $returnValue;
    }
  }

}

