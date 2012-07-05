<?php

/**
 * @file
 */
function islandora_chem_sp_ingest_form($form_state) {
  
  $form = array();

  $form['#attributes'] = array('enctype' => "multipart/form-data");
  
  $form['formats'] = array(
    '#value' => l(t('Supported formats'), 'formats'),
    '#suffix' => '<br/>',
  );
  
  $form['upload'] = array(
    '#type' => 'file',
  );

  $form['structure_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload'),
    '#name' => 'ingest_submit',
  );

  return $form;
}

function islandora_chem_sp_ingest_form_validate($form, &$form_state) {

  $field_name = 'upload';

  if (isset($_FILES['files']) && is_uploaded_file($_FILES['files']['tmp_name'][$field_name])) {

    $file = file_save_upload($field_name);

    if (!$file) {
      form_set_error($field_name, 'Error uploading file.');
      return;
    }

    $form_state['values']['file'] = $file;
  }
  else {
    form_set_error($field_name, 'Error uploading file.');
    return;
  }
}

function islandora_chem_sp_ingest_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_chem_sp', 'FileConversion');
  module_load_include('inc', 'islandora_content_model_forms', 'FOXML');
  module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
  module_load_include('inc', 'islandora_chem_sp', 'chem');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'islandora_chem_sp', 'includes/islandora_chem_sp_batch');

  global $user;

  $file = $form_state['values']['file'];
  $contents = file_get_contents($file->filepath);

  $ext = strrchr($file->filename, '.');
  $ext = substr($ext, 1);

  $allowed_extensions = array(
    'mol',
    'xyz',
    'pdb',
    'cdx',
    'c3d',
    'chm',
    'cif',
    'cml',
    'inp',
    'gam',
    'out',
    'cub',
    'gau',
    'gal',
    'jdx',
    'dx',
    'mop',
    'in',
    'zip',
  );

  if (!in_array($ext, $allowed_extensions) && $ext != NULL) {
    drupal_set_message(t('The file you uploaded is not an allowed type!'), 'warning');
    return;
  }

  if ($ext == 'zip') {
    islandora_chem_sp_batch_ingest($file->filepath);
    return;
  }

  if ($ext == NULL) {
    $start = substr($contents, 0, 20);
    switch ($start) {
      case 'MacSPARTAN \'08 MECHA':
        drupal_set_message(t('This is a MacSpartan output file'));
        $file_name = islandora_chem_sp_convert_spartan_output($file->filepath);
        break;
      case ' OPT B3LYP 6-31G* CO':
        drupal_set_message(t('This is a MacSpartan input file (smol)'));
        $file_name = islandora_chem_sp_convert_spartan($file->filepath);
        break;
      default:
        drupal_set_message(t('I have no idea what this file is!'), 'warning');
    }
  }
  else {
    $file_name = $file->filepath;
  }
  
  islandora_chem_sp_simple_ingest($file_name);

  $form_state['redirect'] = $base_url . "/fedora/repository/{$collection_pid}";
}

function islandora_chem_sp_simple_ingest($file_name, $separate = TRUE) {
  $file_name_array = array();
  $file_name_array['OBJ'] = $file_name;
  
  $title =  basename($file_name);

  $transform = drupal_get_path('module', 'islandora_content_model_forms') . "/transforms/mods_to_dc.xsl";
  $content_model_pid = 'islandora:sp_chem_CM';
  $collection_pid = 'quantumchem:sp_chem_calculations';
  $dsid = 'MODS';
  $pid = Fedora_Item::get_next_PID_in_namespace('quantumchem');
  $username = $user->name;
  $xml = "<mods xmlns=\"http://www.loc.gov/mods/v3\">
  <titleInfo>
    <title>$title</title>
  </titleInfo>
  <name type=\"personal\">
    <namePart>$username</namePart>
    <role>
      <roleTerm authority=\"marcrelator\" type=\"text\">Author</roleTerm>
    </role>
  </name>
</mods>";

  $document = new DOMDocument();
  $document->loadXML($xml);
  $document2 = $document->saveXML();
  $document3 = new DOMDocument();
  $document3->loadXML($document2);

  $collection_policy = CollectionPolicy::loadFromCollection($collection_pid);
  if ($collection_policy !== FALSE) {
    $relationship = $collection_policy->getRelationship();
  }

  $ingest = new IslandoraChem($pid);
  $mol = $ingest->chem2mol(NULL, 'MOL', $file_name, 'mol');
  if ($mol != 1) {
    drupal_set_message(t("Creating an archival version of $file_name failed!"), 'error');
    return;
  }
  $file_name_array['MOL'] = $_SESSION['fedora_ingest_files']['MOL'];
  $cml = $ingest->chem2cml(NULL, 'CML', $file_name, 'xml', $separate);
  if ($cml != 1) {
    drupal_set_message(t("Extracting metadata from $file_name failed!"), 'error');
    return;
  }
  $file_name_array['CML'] = $_SESSION['fedora_ingest_files']['CML'];
  
  $jpg = $ingest->chem2jpg(array('width' => '100', 'height' => '100'), 'TN', $file_name, 'jpg');
  if ($jpg != 1) {
    drupal_set_message(t("Creating the thumbnail for $file_name failed!"), 'error');
    return;
  }
  $file_name_array['TN'] = $_SESSION['fedora_ingest_files']['TN'];
  $medium = $ingest->chem2jpg(array('width' => '250', 'height' => '250'), 'MEDIUM', $file_name, 'jpg');
  if ($medium != 1) {
    drupal_set_message(t("Creating an image of $file_name failed!"), 'error');
    return;
  }

  $file_name_array['MEDIUM'] = $_SESSION['fedora_ingest_files']['MEDIUM'];
  $object = new FOXML($title, $pid, $dsid, $content_model_pid, $collection_pid, $relationship, $file_name_array, $document3, $transform);
  $submit = $object->ingest();
  if (!$submit) {
    drupal_set_message(t("An error occurred when saving the file!"), 'error');
  }
}
