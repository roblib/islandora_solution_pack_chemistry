<?php

function islandora_chem_cron() {

  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'CollectionManagement');
  module_load_include('inc', 'fedora_repository', 'BatchIngest');
  module_load_include('inc', 'islandora_chem_sp', 'FileConversion');

  $directory = '/var/www/drupal/sites/default/files/watch';
  $doNotAdd = array('.', '..', '__MACOSX');

  $zipfile = unzipFiles($directory, $doNotAdd);
  $dirs = array();
  $tmpDir = $directory . '/temp/';
  array_push($dirs, $tmpDir);
  $files = scandir($tmpDir);
  foreach ($files as $file) {
    if (!in_array($file, $doNotAdd)) {
      cleanXYZ($tmpDir . $file);
      $tmpFile = $tmpDir . $file;
//      drupal_set_message('Temp file: ' . $tmpFile);
      createMODS($tmpFile);
    }
    if (is_dir("$tmpDir/$file") & !in_array($file, $doNotAdd)) {

      array_push($dirs, $tmpDir . $file);
    }
  }

  if ($inputs = opendir($tmpDir)) {
    while (FALSE !== ($file_name = readdir($inputs))) {
      if (!in_array($file_name, $doNotAdd) && is_dir($file_name) == FALSE) {
        $ext = strrchr($file_name, '.');
        $base = preg_replace("/$ext$/", '', $file_name);
        $ext = substr($ext, 1);
        if ($ext) {
          $file_list[$base][$ext] = $tmpDir . $file_name;
          drupal_set_message('File name: ' . $file_name);
        }
      }
    }
    closedir($inputs);
  }
  if (!$file_list == NULL) {
    $file_list = array_chunk($file_list, 5, TRUE);
    foreach ($file_list[0] as $label => $object_files) {
      drupal_set_message('Object files: ' . implode(', ', $object_files));
      create_batch_objects($label, 'quantumchem:sp_chem_CM', $object_files, 'quantumchem:sp_chem_calculations', 'quantumchem', '');
      $delete_ext = strrchr(array_shift(array_values($object_files)), '.');
      $delete_base = preg_replace("/$delete_ext$/", '', array_shift(array_values($object_files)));
      $mask = $delete_base . '.*';
      array_map("unlink", glob($mask));
      $delete_base = str_replace(' ', '-', $delete_base);
      $mask = $delete_base . '.*';
      array_map("unlink", glob($mask));
      if (is_dir($tmpDir) && isEmptyDir($tmpDir)) {
        rmdir($tmpDir);
      }
      if (file_exists($zipfile)) {
        unlink($zipfile);
      }
      watchdog('islandora_chem_sp', '@object_files ingested sucessfully', array('@object_files' => implode(', ', $object_files)), WATCHDOG_NOTICE);
    }
  }
}