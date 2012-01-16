<?php

/**
 * @file
 */

/**
 * Hook cron implementation to unzip and ingest files when cron runs
 */
function islandora_chem_cron() {

  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'CollectionManagement');
  module_load_include('inc', 'fedora_repository', 'BatchIngest');
  module_load_include('inc', 'islandora_chem_sp', 'FileConversion');

  $users = array();
  $directories = array();
  $directory = '';
  $root_path=realpath(drupal_get_path('module', 'node').'/../../');

  $query = db_query('SELECT name FROM users');
  while ($result = db_fetch_array($query)) {
    if ($result['name'] != '' && $result['name'] != 'admin') {
      $users[] = $result;
    }
  }
  foreach ($users as $person) {
    $path = $root_path . '/' . file_directory_path() . '/users/' . $person['name'] . '/watch';
    if (!is_dir($path)) {
      mkdir($path, 0775, TRUE);
    }
    $tmp_path = $path . '/temp';
    $directories[] = $path;
  }

//  drupal_set_message('Dirs: ' . implode(', ', $directories));
  $do_not_add = array('.', '..', '__MACOSX');
  
  foreach ($directories as $directory) {
    
    $files = array();
    $file = '';
    $file_list = array();
    $dirs = array();
    
    $zipfile = unzip_files($directory, $do_not_add);
    $tmp_dir = $directory . '/temp/';
    if (is_dir($tmp_dir)) {
      array_push($dirs, $tmp_dir);
      $files = scandir($tmp_dir);
//      drupal_set_message('Files: ' . implode(', ', $files));
      foreach ($files as $file) {
        if (!in_array($file, $do_not_add)) {
          $return = clean_XYZ($tmp_dir . $file);
          $tmp_file = $tmp_dir . $file;
          create_mods($tmp_file);
        }
        if (is_dir("$tmp_dir/$file") & !in_array($file, $do_not_add)) {

          array_push($dirs, $tmp_dir . $file);
        }
      }

      if ($inputs = opendir($tmp_dir)) {
        while (FALSE !== ($file_name = readdir($inputs))) {
          if (!in_array($file_name, $do_not_add) && is_dir($file_name) == FALSE) {
            $ext = strrchr($file_name, '.');
            $base = preg_replace("/$ext$/", '', $file_name);
            $ext = substr($ext, 1);
            if ($ext) {
              $file_list[$base][$ext] = $tmp_dir . $file_name;
            }
          }
        }
        closedir($inputs);
      }
      if (!$file_list == NULL) {
        $file_list = array_chunk($file_list, 5, TRUE);
//        drupal_set_message('File list: ' . implode(', ', $file_list[0]));
        foreach ($file_list[0] as $label => $object_files) {
          create_batch_objects($label, 'quantumchem-sys:sp_chem_CM', $object_files, 'quantumchem-sys:sp_chem_calculations', 'quantumchem', '');
          $delete_ext = strrchr(array_shift(array_values($object_files)), '.');
          $delete_base = preg_replace("/$delete_ext$/", '', array_shift(array_values($object_files)));
          $mask = $delete_base . '.*';
          array_map("unlink", glob($mask));
          $delete_base = str_replace(' ', '-', $delete_base);
          $mask = $delete_base . '.*';
          array_map("unlink", glob($mask));
          if (is_dir($tmp_dir) && is_empty_dir($tmp_dir)) {
            rmdir($tmp_dir);
          }
          if (file_exists($zipfile)) {
            unlink($zipfile);
          }
          watchdog('islandora_chem_sp', '@object_files ingested sucessfully', array('@object_files' => implode(', ', $object_files)), WATCHDOG_NOTICE);
        }
      }
    }
  }
}