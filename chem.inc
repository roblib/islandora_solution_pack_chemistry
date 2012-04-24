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
    module_load_include('inc', 'islandora_chem_sp', 'FileConversion');
    drupal_add_css(drupal_get_path('module', 'islandora_chem_sp') . 'islandora_chem_sp.css');
    $pid = $this->pid;
    $object_profile = islandora_chem_sp_object_profile($pid);
    $uid = islandora_chem_sp_get_uid($object_profile['owner']);
    $object_user = user_load($uid);
    $contributor = $object_user->profile_firstname . ' ' . $object_user->profile_lastname;
    if ($contributor == NULL) {
      if ($object_profile['owner'] != NULL) {
        $contributor = $object_profile['owner'];
      }
      else {
        $contributor = 'Unknown';
      }
    }
    $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');
    $permission = FALSE;
    $tabset = array();

    $tabset['first_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Description'),
    );

    $tabset['first_tab']['tabs'] = array(
      '#type' => 'tabset',
    );

//    if ($permission == FALSE) {
//      $tabset['first_tab']['tabs']['deny'] = array(
//        '#value' => t('You do not have permission to view this object!'),
//      );
//      return $tabset;
//    }
//    
//    Create path variables 
    $path = drupal_get_path('module', 'Fedora_Repository');
    $fullPath = base_path() . $path;
    $content = "";
    $pathTojs = drupal_get_path('module', 'islandora_chem_sp') . '/js/';
//    Show the medium sized image of the structure at the top of the view
//    $content1 = '<img src="http://192.168.200.128:8080/fedora/objects/quantumchem:8272/datastreams/MEDIUM/content" />';
    $content1 = '<img src="' . $base_url . '/fedora/repository/' . $this->pid . '/MEDIUM/MEDIUM.jpg" />';
//    Load the XML from the CML and DC datastreams
    $item = new Fedora_Item($pid);
    $xmlstr = $item->get_datastream_dissemination("CML");
    $xmlstr2 = $item->get_datastream_dissemination("DC");
//    If the CML datastream exists then display the relevant information.
//    It's unlikely that the DC stream will exist if the CML one doesn't
    $title = '';
    if (!$xmlstr == NULL) {
//      Pull out the elements from the XML
      $xml2 = new SimpleXMLElement($xmlstr2);
      $ns_xml2 = $xml2->children('dc', TRUE);
      $title = $ns_xml2->title;
//      $contributor = $ns_xml2->contributor;
      $author = $ns_xml2->contributor;
      $xml = new SimpleXMLElement($xmlstr);
      $ns_xml = $xml->children();
      $desc = $ns_xml->molecule->attributes()->title;
      $mw = $ns_xml->xpath('//scalar');
      $formula = $ns_xml->molecule->formula->attributes()->concise;
      $smiles = $ns_xml->xpath("//alternative[@id='sm1']");
      $inchi = $ns_xml->molecule->identifier->inchi;
      // Loop through each of the inchikeys as there may be multiple in a cluster
      foreach ($ns_xml->molecule->identifier->inchikey as $key) {
        $inchi_key[] = $key;
      }
      $mw2 = $mw[0][0];

//      The formula is stored with spaces in to conform to the CML standard so we need to get rid of them
//      Also make the numbers subscript 
      $formula2 = formula_cleanup($formula);

//      Clean up IUPAC name as it's usually ugly. If there isn't one then deal with it gracefully
      $iupac = $ns_xml->molecule->name;
      $iupac = ucfirst(strtolower(ltrim($iupac)));
      if ($iupac == NULL) {
        $iupac = 'Not found';
      };

      $synonym = $ns_xml->xpath("//alternative[@id='s1']");
      if ($synonym[0] == '') {
        $synonym[0] = 'None found';
      };

      $fragments = $ns_xml->xpath("//alternative[@id='f1']");

      $other_list = array();

      // Again it is possible to have more than once inchi key if the object is a cluster so we have to query for each one
      foreach ($inchi_key as $key) {
        $solr_url = variable_get('fedora_solr_search_url', 'http://localhost:8080/solr');
        $solr_query = $solr_url . '/select?q=cml.inchikey:' . $key . '&fl=PID,dc.title,dc.contributor&rows=20';
        $solr_result = file_get_contents($solr_query);
        $inchi_search = new SimpleXMLElement($solr_result);
        $pid_results = $inchi_search->result;
        $rows2 = array();

        foreach ($pid_results->children() as $child) {
          $other_title = l($child->arr[1]->str . ' ', $base_url . '/fedora/repository/' . $child->str);
          $other_researcher = $child->arr[0]->str;
//          $other_date = 'Date';
          if ($child->str != $this->pid) {
            $rows2[] = array($other_title, $other_researcher, $other_date);
          }
        }
      }
//      Get the URL to chemspider if it exists and create a link from it
      $spider_url = $ns_xml->molecule->metadata->attributes()->content;

      if ($spider_url != '') {
        $spider_entry = l($spider_url, $spider_url);
      }
      else {
        $spider_entry = 'Not found';
      }

      if ($contributor == 'Unknown') {
        $contributor_array = array('Contributor', 'Unknown');
      }
      else {
        $contributor_array = array('Contributor', l($contributor, 'user/' . $uid) . ' | ' . l('Search', $base_url . '/islandora/solr/search/' . preg_replace("/\([^\)]+\)/", "", $contributor) . '/-/dismax'));
      }

//      Create a table to display all the data nicely
      $header = array();
      $rows = array(
        array('Title', $title),
        $contributor_array,
        array('IUPAC name', $iupac),
        array('Synonyms', html_entity_decode($synonym[0])),
        array('Molecular weight', $mw2),
        array('Molecular formula', $formula2),
        array('Canonical smiles', $smiles[0]),
        array('URL', $spider_entry),
      );

      $header2 = array(
        'Title',
        'Researcher',
//        'Date created',
      );

      $header_extra = array();

      $rows_extra = array(
        array('Inchi', '<abbr title="International Chemical Identifier">' . $inchi . '</abbr>'),
        array('Inchikey(s)', implode(', ', $inchi_key)),
        array('Fragments', $fragments[0]),
        array('File description', $desc),
      );

      $tabset['first_tab']['tabs']['summary'] = array(
        '#type' => 'tabpage',
        '#title' => t('Summary'),
        '#content' => $content1,
        '#weight' => 1,
      );

      if (!empty($rows2)) {
        $tabset['first_tab']['tabs']['summary']['related'] = array(
          '#value' => '<b>Related entries</b></br>' . theme('table', $header2, $rows2),
          '#weight' => 1,
        );
      }
      else {
        $tabset['first_tab']['tabs']['summary']['related'] = array(
          '#value' => '<b>Related entries</b></br><p>None found!</p>',
          '#weight' => 1,
        );
      }
    }
    else {
      $content1 .= '<div id="title">No XML datastream found</div>';
    }

    drupal_add_css($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb.css');
    drupal_add_js($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb-libs.js');
    drupal_add_js($pathTojs . 'ChemDoodleWeb/install/ChemDoodleWeb.js');

//    Grab the structure file and clean it up so that it displays in the JS viewer
//    $file = trim(file_get_contents($fedora_url . '/objects/' . $this->pid . '/datastreams/MOL/content'));
    $file = trim($item->get_datastream_dissemination("MOL"));
//    $collectionHelper_mol = new CollectionClass();
//    $file = trim($collectionHelper_mol->getStream($this->pid, "MOL"));
    $file = mol_clean($file);

//    Put in the JS for the viewer and load the MOL file
    $content2 = '<script>
                    var transformBallAndStick = new ChemDoodle.TransformCanvas3D("transformBallAndStick", 250, 250);
                    transformBallAndStick.specs.set3DRepresentation("Ball and Stick");
                    transformBallAndStick.specs.backgroundColor = "black";
                    var molFile = "' . $file . '";
                    var molecule = ChemDoodle.readMOL(molFile, 1);
                    transformBallAndStick.loadMolecule(molecule);
                 </script>';

    $tabset['first_tab']['tabs']['summary']['table'] = array(
      '#value' => theme('table', $header, $rows),
      '#weight' => 0,
    );

    $tabset['first_tab']['tabs']['summary']['extra'] = array(
      '#type' => 'fieldset',
      '#title' => t('Extra information'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 2,
    );
    
    $tabset['first_tab']['tabs']['summary']['extra']['table2'] = array(
      '#value' => theme('table', $header_extra, $rows_extra),
      '#weight' => 2,
    );

    $tabset['first_tab']['tabs']['view'] = array(
      '#type' => 'tabpage',
      '#title' => t('3D view'),
      '#content' => $content2,
      '#weight' => 2,
    );
//    Display the raw output file as text if it exists
    $xmlstr_output3 = $item->get_datastream_dissemination("OBJ");
//    $collectionHelper_output3 = new CollectionClass();
//    $xmlstr_output3 = $collectionHelper_output3->getStream($this->pid, "OBJ");

    $output = '';

    $downloadLink = $base_url . '/download/' . $this->pid;
    $output .= <<<JS
    <script>
                  function download() { 
        window.open("$downloadLink", 'Download');
        }   
                 </script></BR>
JS;

    $output .= '<form><input type="button" value="Download" onclick="download()" /></form></BR>';

    if (!$xmlstr_output3 == NULL) {
//      $output .= str_replace("\n", '</BR>', file_get_contents($fedora_url . '/objects/' . $this->pid . '/datastreams/OBJ/content'));
      $output .= str_replace(array("\n", '$$'), array('</BR>', '**'), $xmlstr_output3);
      $content3 = $output;
    }
    else {
      $content3 = '<div id="title">No OBJ datastream found</div>';
    }

    $tabset['first_tab']['tabs']['output'] = array(
      '#type' => 'tabpage',
      '#title' => t('Output file'),
      '#prefix' => '<div class="code_window">',
      '#suffix' => '</div>',
      '#content' => $content3,
      '#weight' => 3,
    );
//    Display the raw input file as text if it exists
    $xmlstr_input = $item->get_datastream_dissemination("INPUT");
//    $collectionHelper_input = new CollectionClass();
//    $xmlstr_input = $collectionHelper_input->getStream($this->pid, "INPUT");

    if (!$xmlstr_input == NULL) {
      $content4 = str_replace("\n", '</BR>', $xmlstr_input);

      $tabset['first_tab']['tabs']['input'] = array(
        '#type' => 'tabpage',
        '#title' => t('Input file'),
        '#content' => $content4,
        '#weight' => 4,
      );
    }
//    Display the raw code as text if it exists
    $xmlstr_code = $item->get_datastream_dissemination("CODE");
//    $collectionHelper_code = new CollectionClass();
//    $xmlstr_code = $collectionHelper_code->getStream($this->pid, "CODE");

    if (!$xmlstr_code == NULL) {
      module_load_include('php', 'geshifilter', '/geshi/geshi');
      $geshi = new GeSHi($xmlstr_code, 'php');
//      $content5 = str_replace("\n", '</BR>', SyntaxHighlight::process($xmlstr_code));
      $content5 = str_replace("\n", '</BR>', $geshi->parse_code());

      $tabset['first_tab']['tabs']['code'] = array(
        '#type' => 'tabpage',
        '#title' => t('Code'),
        '#content' => '<code>' . $content5 . '</code>',
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
    module_load_include('inc', 'islandora_chem_sp', 'FileConversion');
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
    clean_XYZ($file);
//  Call openbabel and convert the file to MOL. (No longer needed - Create the 3D structure so that it looks good in the 3D viewer)
    if (!file_exists($file . "svg")) {
      system("obabel \"$file\" --title -omol -xw -O\"$filename\"", $returnValue);
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
    module_load_include('inc', 'islandora_chem_sp', 'FileConversion');
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
    $height = $parameterArray['height'];
    $width = $parameterArray['width'];
    $returnValue = TRUE;
    clean_XYZ($file);
//  OpenBabel won't convert directly to jpg so we need to go via svg
    if (!file_exists($file . "png")) {
      system("obabel \"$file\" -opng -O\"$file\".png --title -xw -c", $returnvalue);
    }
    else {
      $returnValue = '0';
    }
    $returnValue = '0';
//  Once the svg is created we need to convert it to jpg using imagemagick
    if (!file_exists($filename)) {
      system("convert \"$file\".png -resize \"$width\"x\"$width\" -gravity center -modulate 100,140 -extent \"$width\"x\"$width\" \"$filename\"", $returnValue);
      unlink($file . '.png');
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
    module_load_include('inc', 'islandora_chem_sp', 'FileConversion');
    $file_suffix = '_' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
//    drupal_set_message('File: ' . $file);
    clean_XYZ($file);
//  Run various system calls to generate the data
    if (!file_exists($filename)) {
      exec("obprop \"$file\"", $return, $returnValue1);
      if ($returnValue1 != '1') {
        drupal_set_message(t('obprop failed to run properly'), 'error');
        return $returnValue1;
      }
      exec("obabel \"$file\" -oinchi --separate -xK", $inchi_key, $returnValue2);
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
      switch ($returnValue4) {
        case 2:
          drupal_set_message(t('checkmol could not find the specified file'), 'warning');
          break;
        case 3:
          drupal_set_message(t('checkmol did not recognise the file type'), 'warning');
          break;
      }

      if ($returnValue4 != 0) {
        $fragment = NULL;
      }

      exec("obabel \"$file\" -osmi -O /tmp/checkmol.smi -xc", $return, $returnValue5);
      if ($returnValue5 != '0') {
        drupal_set_message(t('The smiles string could not be calculated'), 'error');
        return $returnValue5;
      }

//      This calls a python script to generate the molecular fingerprint
      exec('/opt/ActivePython-2.7/bin/python2.7 ' . drupal_get_path('module', 'islandora_chem_sp') . '/indigo-python-api-1.0.0-linux/fingerprint.py /tmp/checkmol.smi', $fingerprint, $returnValue6);
      if ($returnValue6 != '0') {
        drupal_set_message(t('The molecular fingerprint could not be calculated'), 'error');
        return $returnValue6;
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

      if (islandora_chem_sp_url_exists('http://rdf.chemspider.com/')) {

        $url_headers = get_headers($rdf_url);

        if ($url_headers[0] == 'HTTP/1.1 302 Found') {
          $rdf->load($rdf_url);
          $trivial = $rdf->getElementsByTagName('Description')->item(13)->nodeValue;
//    The URL to the chemspider entry is also generated
          if ($rdf->getElementsByTagName('Description')->item(1)) {
            $spider_url = $rdf->getElementsByTagName('Description')->item(1)->getAttribute('rdf:about');
          }
          else {
            $trivial = 'No synonyms available';
          }
        }
        else {
          $trivial = 'Structure not found';
          $spider_url = '';
        }
      }
      else {
        $trivial = 'Structure not found';
        drupal_set_message(t('Could not connect to http://rdf.chemspider.com/'), 'warning');
      }

      $fragment = trim(str_replace(';', ' ', $fragment[0]));
      $smiles = file_get_contents('/tmp/checkmol.smi');
      $smiles = explode("\t", $smiles);

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

      $xmltrivial = $dom->createElement("name", ucfirst(ltrim($trivial)));
      $xmltrivial->setAttribute("id", "n0");
      $xmltrivial->setAttribute("convention", "IUPAC:trivial");
      $xmlname->appendChild($xmltrivial);

//    Synonyms are retrieved from the pubchem/NCI db and added to the XML for searching
      if (isset($inchi_key[0])) {
        $url = 'http://cactus.nci.nih.gov/chemical/structure/' . urlencode($inchi_key[0]) . '/names/xml';

        if (islandora_chem_sp_url_exists('http://cactus.nci.nih.gov')) {
          $iupac_xml = new DOMDocument();
          $iupac_xml->load($url);
          $nodes = $iupac_xml->getElementsByTagName('item');
          $nodeno = $nodes->length;

          $synonym = '';

          for ($i = 0; $i < $nodeno; $i++) {
            $iupac = $iupac_xml->getElementsByTagName('item')->item($i)->nodeValue;
            $iupac = htmlentities(ucfirst(strtolower($iupac)));
            $synonym .= ', ' . $iupac;
          }

          $xmlsynonym = $dom->createElement("alternative", ltrim($synonym, ','));
          $xmlsynonym->setAttribute("id", "s1");
          $xmlsynonym->setAttribute("type", "synonym");
          $xmlname->appendChild($xmlsynonym);
        }
        else {
          drupal_set_message(t('Could not connect to @url', array('@url' => $url)), 'warning');
        }
      }
      else {
        $xmlsynonym = $dom->createElement("alternative", 'None found');
        $xmlsynonym->setAttribute("id", "s1");
        $xmlsynonym->setAttribute("type", "synonym");
        $xmlname->appendChild($xmlsynonym);
      }

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

      $xmlfingerprint = $dom->createElement("alternative", implode(', ', $fingerprint));
      $xmlfingerprint->setAttribute("id", "fp1");
      $xmlfingerprint->setAttribute("type", "fingerprint");
      $xmlname->appendChild($xmlfingerprint);

      $xmlsmiles = $dom->createElement("alternative", $smiles[0]);
      $xmlsmiles->setAttribute("id", "sm1");
      $xmlsmiles->setAttribute("type", "smiles");
      $xmlname->appendChild($xmlsmiles);

      $xmlidentifier = $dom->createElement("identifier");
      $xmlname->appendChild($xmlidentifier);

      $xmlinchi = $dom->createElement("inchi", $inchi_value);
      $xmlidentifier->appendChild($xmlinchi);

      foreach ($inchi_key as $key) {
        $xmlinchikey = $dom->createElement("inchikey", $key);
        $xmlidentifier->appendChild($xmlinchikey);
      }

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
      return TRUE;
    }
    else {
      return $returnValue;
    }
  }

  function showMADS() {
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

    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "MADS");
    $xml = new SimpleXMLElement($xmlstr);

//    var_dump($xml);

    $names = $xml->authority->name->namePart;
    $fullName = $names[0] . ' ' . $names[1];
    $interests = $xml->authority->topic;
    $organization = $xml->affiliation->organization;
    $position = $xml->affiliation->position;
    $email = $xml->affiliation->email;

    $header = array();

    $rows = array(
      array('Name', $fullName),
      array('Organization', l($organization, 'islandora/solr/search/' . $organization . '/-/dismax')),
      array('Position', $position),
      array('Email', $email),
      array('Interests', l($interests, 'islandora/solr/search/' . $interests . '/-/dismax')),
    );

    $imgPath = $base_path . drupal_get_path('module', 'islandora_chem_sp') . '/images/gnome_stock_person.png';

    $tabset['first_tab']['tabs']['image'] = array(
      '#value' => '<img src = "' . $imgPath . '"/>',
    );

    $tabset['first_tab']['tabs']['table'] = array(
      '#value' => theme('table', $header, $rows),
    );

    $calculations_list = array();
    $solr_url = variable_get('fedora_solr_search_url', 'http://localhost:8080/solr');
    $solr_query = $solr_url . '/select?q=dc.contributor:"' . urlencode($fullName) . '"&fl=PID,dc.title,cml.name';
    $solr_result = file_get_contents($solr_query);
    $calc_search = new SimpleXMLElement($solr_result);
    $pid_results = $calc_search->result;

    $rows2 = array();

    foreach ($pid_results->children() as $child) {
      $calc_title = l($child->arr[1]->str . ' ', $base_url . '/fedora/repository/' . $child->str);
      $calc_name = $child->arr[0]->str;
      $calc_date = 'Date';
//        var_dump($this->pid);
//        var_dump($child->str);
      if ($child->str != $this->pid) {
        $rows2[] = array($calc_title, ucfirst(ltrim($calc_name)), $calc_date);
      }
    }

    $header2 = array('Title', 'Name', 'Date');

    $content1 = "</BR><B>Associated calculations</B></BR>";
    if (!empty($rows2)) {
      $content1 .= theme('table', $header2, $rows2);
    }
    else {
      $content1 .= "</BR>None found";
    }

    $tabset['first_tab']['tabs']['related'] = array(
      '#value' => $content1,
    );

    return $tabset;
  }

  public function createResearcher() {

    $dsid = 'TN';

    $modulePath = drupal_get_path('module', 'islandora_chem_sp');

    $filename = '/var/www/drupal/sites/all/modules/islandora_solution_pack_chemistry/images/gnome_stock_person.png';

    if (file_exists($filename)) {
      $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
      return TRUE;
      print "Ingest succeeded!";
    }
    else {
      return FALSE;
      print "Ingest failed!";
    }
  }

}