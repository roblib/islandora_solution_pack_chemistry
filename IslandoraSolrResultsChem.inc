<?php

/*
 * contains methods to search solr and display results.  depends on Apache_Solr_Php client.
 */

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

/**
 * Extention of IslandoraSolrResults for templating purposes.
 * This overrides the displayResults function to provide an alternate display type.
 */
class IslandoraSolrResultsChem extends IslandoraSolrResults {

  static $facetSeparator = '~'; //used to separate facets in url
  static $slashReplacement = '~slsh~'; // a pattern used to replace / in url's the slash breaks drupal clean url's
  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;

  function IslandoraSolrResultsTable() {
    parent::__construct();
  }

  /**
   * Outputs results in unified tabular format.
   *
   * This function collects all field names from all results in the 
   * resultset and constructs a single table with said field names in 
   * a unified header row, rather than outputting each result as a 
   * miniature horizontal table.
   *
   * @param $results
   *   A solr resultset object.
   *
   * @return
   *   html output for the resultset. Note: we currently create this 
   *   output manually, should refactor to use drupal forms api.
   */
  function displayResultsChem($solrQueryProcessor, $title = "Search Results", $output = '') {

    $apacheSolrResult = $solrQueryProcessor->solrResult;
    $total = (int) $apacheSolrResult->response->numFound;
    $end = min(($solrQueryProcessor->solrLimit + $solrQueryProcessor->solrStart), $total);

    // Check for empty resultset
    if ($total === 0) {
      $output = "<h2>Search Results</h2>";
      $output .= "<div>Sorry, but your search returned no results.</div>";
      return $output;
    }

    if (!$_SESSION['mol'] == NULL) {
      $path = drupal_get_path('module', 'islandora_chem_sp');
      $path_to_js = $path . '/js/';
      drupal_add_css($path . '/islandora_chem_sp_search.css');
      drupal_add_css($path_to_js . 'ChemDoodleWeb/install/sketcher/jquery-ui-1.8.7.custom.css');
      drupal_add_js($path_to_js . 'ChemDoodleWeb/install/ChemDoodleWeb-libs.js');
      drupal_add_js($path_to_js . 'ChemDoodleWeb/install/ChemDoodleWeb.js');
      drupal_add_js($path_to_js . 'ChemDoodleWeb/install/sketcher/jquery-ui-1.8.7.custom.min.js');
      drupal_add_js($path_to_js . 'ChemDoodleWeb/install/sketcher/ChemDoodleWeb-sketcher.js');

      $mol = $_SESSION['mol'];
      $_SESSION['mol'] = '';
      $find = array("\n", "\r", "http://");
      $replace = array('\n', '', '');
      $mol = str_replace($find, $replace, $mol);

      $search_structure .= <<<JS
  <script>
  var viewmol = new ChemDoodle.ViewerCanvas('viewmol', 100, 100);
  viewmol.specs.backgroundColor = null;
  viewmol.specs.bonds_width_2D = .6;
  viewmol.specs.bonds_saturationWidth_2D = .18;
  viewmol.specs.bonds_hashSpacing_2D = 2.5;
  viewmol.specs.atoms_font_size_2D = 10;
  viewmol.specs.atoms_font_families_2D = ['Helvetica', 'Arial', 'sans-serif'];
  viewmol.specs.atoms_displayTerminalCarbonLabels_2D = true;
  viewmol.specs.atoms_useJMOLColors = true;
  var caffeineMolFile = "$mol";
  var caffeine = ChemDoodle.readMOL(caffeineMolFile);
  viewmol.loadMolecule(caffeine);
</script>
JS;

//  $headers = array();
//  $rows = array(
//    array(t('This is the structure you searched for:'), $search_structure,),
//  );
//  
//  $table = theme('table', $headers, $rows);
      $table = '<div class="search-text">' . t('This is the structure you searched for:') . '</div>';
      $table .= '<div class="search-mol">' . $search_structure . '</div>';

      $output .= $table;
    }

    // Initialize drupal-style pager
    islandora_solr_search_pager_init($total, $solrQueryProcessor->solrLimit);

    // Get secondary display profiles
    $secondary_display_profiles = module_invoke_all('islandora_solr_secondary_display');
    // How about this for an expected return?
    // return array(
    // 'machine-name' = array(
    //   'name' => 'Human Readable Name',
    //   'module' => 'module_name',
    //   'file' => 'FileName.inc',
    //   'class' => 'ClassName',
    //   'function' => 'function_name',
    //   'description' => 'A description of the display profile',
    //   'logo' => 'Some text, some <html>, an <img>, etc used to link to this output,
    // );
    // check if
    // if the islandora_solr_search admin form has never been saved, then
    // the variable 'islandora_solr_secondary_display' won't be set yet.
    $secondary_array = variable_get('islandora_solr_secondary_display', array());

    // count number of elements in array
    $array_count = count($secondary_array);
    // count number of disabled displays
    $i = 0;
    foreach ($secondary_array as $key => $value) {
      if ($value === 0) {
        $i++;
      }
    }

    // if number of elements is the same as the number of disabled displays,
    // this means that there are no displays enabled. 
    if ($array_count != $i) {

      $output .= "<div id='islandora_solr_secondary_display_profiles'>";

      foreach ($secondary_array as $name => $status) {
        if ($status === $name) {

          $description = $secondary_display_profiles[$name]['description'];
          $logo = $secondary_display_profiles[$name]['logo'];

          $attributes = array('title' => $description);
          $query = array();
          $query[] = drupal_query_string_encode(array('solr_profile' => $name));
          $output .= l($logo, $_GET['q'], array('attributes' => $attributes, 'html' => TRUE, 'query' => count($query) ? implode('&', $query) : NULL));
        }
      }

      $output .= "</div>";
    }

    $output .= '<h2>' . $title . '</h2>';
    $output .= '<div id="islandora_solr_result_count">(' . ($solrQueryProcessor->solrStart + 1) . ' - ' . $end . ' of ' . $total . ')</div>';
    $output .= '<div class="content">';

//    drupal_set_message(print_r($secondary_display_profiles,1));

    /*
     * Then, here, we need to build links to the current query under the secondary query display
     * profile for each such profile which uses the hook.
     *
     * The link should basically be $theCurrentURL with &proile=<profile_name> tacked on.
     *
     * Also, perhaps there's no difference between primary and secondary profiles?
     */

    $output .= $this->printResultsChem($solrQueryProcessor->solrResult);

    // Render Drupal pager (bottom of search results)
    $output .= theme('pager', NULL, $solrQueryProcessor->solrLimit, 0, NULL, 5);


    $output .= "</div>"; // class = "content"
    return $output;
  }

  function printResultsChem($results) {

    $items = array();
    $type = "ol";
    $title = NULL;
    $recordStart = $results->response->start;
    $limitResults = variable_get('islandora_solr_search_limit_result_fields', 0);
    $highlights = $results->highlighting;
    foreach ($highlights as $highlight) {
      $fields = get_object_vars($highlight);
      $keys = array_keys($fields);
      if (count($keys) > 0) {
        foreach ($highlight->$keys[0] as $body) {
          //drupal_set_message("$keys[0]  $body");
        }
      }
      //drupal_set_message($highlight->$keys[0]);
    }

    global $base_url;
    if (empty($results)) {
      return "no results";
    }

    foreach ($results->response->docs as $doc) {
      $rows = array();
      $row = 0;

//      $rows[$row][] = array(
//          'data' => '',
//          'header' => TRUE,
//        );

      $tn_url = $base_url . '/fedora/repository/' . htmlspecialchars($doc->PID, ENT_QUOTES, 'utf-8') . '/TN';
      $tn2_url = drupal_get_path('module', 'islandora_chem_sp') . '/images/gnome_stock_person.png';
//      Check if the TN datastream exists, if it does then display the thumbnail in the search results.
      if (@file_get_contents($tn_url)) {
        $tn = '<a href="' . $base_url . '/fedora/repository/' . htmlspecialchars($doc->PID, ENT_QUOTES, 'utf-8') . '"><img src="' . $tn_url . '" /></a>';
        $rows[][] = $tn;
        $row++;
      }
      else {
        $tn = '<a href="' . $base_url . '/fedora/repository/' . htmlspecialchars($doc->PID, ENT_QUOTES, 'utf-8') . '"><img src="' . $base_url . '/' . $tn2_url . '" /></a>';
        $rows[][] = $tn;
        $row++;
      }
      foreach ($doc as $field => $value) {

        if ($limitResults && empty($this->resultFieldArray[$field])) {
          continue;
        }

        $translated_field_name = isset($this->allSubsArray[$field]) ? $this->allSubsArray[$field] : $field;
        $rows[$row][] = array(
          'data' => $translated_field_name,
          'header' => TRUE,
        );
        if (is_array($value)) {
          $value = implode(", ", $value);
          if (strlen($value) > 250) {
            $value = substr($value, 0, 250) . '...';
          }
        }

        if ($field == 'PID') {
          $l = l($value, 'fedora/repository/' . htmlspecialchars($value, ENT_QUOTES, 'utf-8')) . '</BR>';
          $rows[$row][] = $l;
        }
        elseif ($field == 'cml.formula') {
          $value = str_replace(' ', '', $value);
          $value = preg_replace('/([0-9]*)/', '<sub>$1</sub>', $value);
          $rows[$row][] = $value;
        }
        else {
          if (strlen($value) > 250) {
            $value = substr($value, 0, 250) . '...';
          }
          $rows[$row][] = ucfirst(ltrim($value));
        }
        $row++;
      }
      $items[] = theme_table(NULL, $rows, array('class' => 'islandora_solr_search_results_object'));
    }
    $output = theme_item_list($items, $title, $type, array('class' => 'stuff', 'start' => ($recordStart + 1)));
    if (variable_get('islandora_solr_search_debug_mode', 0)) { // debug dump
      $results_r .= "<pre>Results: " . print_r($results, TRUE) . "</pre>";
      $fieldset_r = array(
        '#title' => t("Raw Results"),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#value' => $results_r,
      );
      $output .= theme('fieldset', $fieldset_r);
    }
    return $output;
  }

}