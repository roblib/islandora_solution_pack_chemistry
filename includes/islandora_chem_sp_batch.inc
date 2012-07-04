<?php

/**
 * @file
 */
function islandora_chem_sp_batch_ingest($file) {
  global $base_path;
  $tmp_dir = file_directory_temp() . '/' . substr(str_shuffle(str_repeat('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789', 5)), 0, 5);
  mkdir($tmp_dir);
  $tmp_file = $tmp_dir . '/temp.zip';
  if (copy($file, $tmp_file)) {
    unlink($file);
  }

  try {
    chdir(absolute_path_to_drupal() . '/' . $tmp_dir);
    $command = 'unzip ' . absolute_path_to_drupal() . '/' . $tmp_file;
    exec($command, $output, $return_var);
  } catch (Exception $e) {
    drupal_set_message(t("Unable to unzip file - $output - $e"), 'error');
  }
  unlink(absolute_path_to_drupal() . '/' . $tmp_file);
  islandora_chem_sp_recursive_directory_delete(absolute_path_to_drupal() . '/' . $tmp_dir . '/__MACOSX');

  $batch = array(
    'finished' => 'islandora_chem_sp_batch_finished',
    'title' => t('Ingesting batch'),
    'init_message' => t('Batch ingesting is starting.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Batch ingest has encountered an error.'),
    'file' => drupal_get_path('module', 'islandora_chem_sp') . '/includes/islandora_chem_sp_batch.inc',
    'operations' => array(),
  );

  $it = new RecursiveDirectoryIterator(absolute_path_to_drupal() . '/' . $tmp_dir);
  $file_array = array();
  foreach (new RecursiveIteratorIterator($it) as $file) {
    drupal_set_message('File: ' . basename($file));
    if (!in_array(basename($file), array('.', '..', '.DS_Store'))) {
      $file_array[] = (string) $file;
    }
  }
  foreach ($file_array as $file) {
    $batch['operations'][] = array('islandora_chem_sp_batch_process', array($file));
  }
  batch_set($batch);
}

function islandora_chem_sp_batch_process($file, &$context) {
  module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'islandora_chem_sp', 'chem');
  module_load_include('inc', 'islandora_content_model_forms', 'FOXML');
  module_load_include('inc', 'islandora_chem_sp', 'islandora_chem_sp_blocks');

  $title = basename($file);

  $context['message'] = t('Ingesting file ' . $title);

  $file_name = strstr($file, 'sites/');

  islandora_chem_sp_simple_ingest($file_name);
}

/**
 * Get the true path to the root of the Drupal site.
 * Better than using DOCUMENT_ROOT and base_path().
 */
function absolute_path_to_drupal() {
  static $absolute_path_to_drupal = NULL;

  if ($absolute_path_to_drupal === NULL) {
// Get the absolute path to this file:
    $dir = rtrim(str_replace('\\', '/', dirname(__FILE__)), '/');
    $parts = explode('/', $dir);

// Iterate up the directory hierarchy until we find the website root:
    $done = FALSE;
    do {
// Check a couple of obvious things:
      $done = is_dir("$dir/sites") && is_dir("$dir/includes") && is_file("$dir/index.php");
      if (!$done) {
// If there's no more path to examine, we didn't find the site root:
        if (empty($parts)) {
          $absolute_path_to_drupal = FALSE;
          break;
        }
// Go up one level and look again:
        array_pop($parts);
        $dir = implode('/', $parts);
      }
    } while (!$done);

    $absolute_path_to_drupal = $dir;
  }

  return $absolute_path_to_drupal;
}

function islandora_chem_sp_recursive_directory_delete($dir) {
  if (!file_exists($dir))
    return TRUE;
  if (!is_dir($dir))
    return unlink($dir);
  foreach (scandir($dir) as $item) {
    if ($item == '.' || $item == '..')
      continue;
    if (!islandora_chem_sp_recursive_directory_delete($dir . DIRECTORY_SEPARATOR . $item))
      return FALSE;
  }
  return rmdir($dir);
}