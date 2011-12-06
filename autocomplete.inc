<?php

/**
 * @file
 * Php you suck so badly, the array_filter function should just take extra parameters like array_walk...
 */
class AutoCompleteFilterChem {

  protected $string;

  function __construct($string) {
    $this->string = strtolower($string);
  }

  function __invoke($content_model) {
    return (preg_match("/^{$this->string}/i", $content_model) !== 0) ? TRUE : FALSE;
  }

}

/**
 * Autocomplete researchers from a solr query..
 * 
 * @param string $string The string of characters to match with the results
 * @return string List of results in JSON format
 */
function islandora_content_model_forms_autocomplete_researchers($string = '') {
  $content_models = islandora_chem_sp_get_values('rels.hasModel:researcher_CM', 'dc.title');
  // @todo find an alternative solution to array filter because of earlier versions 5.2
  $output = array_filter($content_models, new AutoCompleteFilterChem($string));
  $output[$string] = $string;
//  var_dump($output);
  return drupal_json($output);
}

/**
 * Autocomplete institutions..
 * 
 * @param string $string The string of characters to match with the results
 * @return string List of results in JSON format
 */
function islandora_content_model_forms_autocomplete_institutions($string = '') {
  $content_models = islandora_chem_sp_get_values('rels.hasModel:researcher_CM', 'mads.organization');
  // @todo find an alternative solution to array filter because of earlier versions 5.2
  if ($content_models == NULL) {
    drupal_set_message('Solr is not responding!', 'error');
    break;
  }
  else {
  $output = array_filter($content_models, new AutoCompleteFilterChem($string));
  $output[$string] = $string;
  return drupal_json($output);
  }
}

/**
 *
 * @param type $query Criteria to search for as a solr query
 * @param type $results The field that you want returned
 * @return type An array of results with the key and element being the same 
 */
function islandora_chem_sp_get_values($query, $results) {
  
  $solr_url = variable_get('fedora_solr_search_url', 'http://localhost:8080/solr');
  
  $query = file_get_contents($solr_url . '/select?q=' . $query . '&fl=' . $results);
  
  $result = new SimpleXMLElement($query);
  $researchers = $result->result;
//  var_dump($researchers);
  $list = array();
  
  foreach ($researchers->children() as $researcher) {
    $value = (string)$researcher->arr[0]->str;
    $list[$value] = $value;
  }
//  var_dump($list);
  return $list;
}