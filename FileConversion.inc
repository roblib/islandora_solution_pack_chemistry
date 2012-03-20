<?php

/**
 * @file
 */

/**
 * Some XYZ files lack the proper headers, this function adds
 * the required data
 * @param type $file
 * @return type
 */
function clean_XYZ($file) {
  $return_value = FALSE;
  $ext = strrchr($file, '.');
  $base = preg_replace("/$ext$/", '', $file);
  $ext = substr($ext, 1);
  drupal_set_message('Extension is: ' . $ext);
  if ($ext == 'xyz') {
    $xyz = file($file);
//    drupal_set_message('First character is: ' . $xyz[0][0]);
    if (check_numeric($xyz[0][0])) {
//            drupal_set_message('Is numeric');
    }
    if (!check_numeric($xyz[0][0])) {
//      drupal_set_message('Cleaning file');
      $fp2 = fopen($file, 'w');
      $atoms = count($xyz);
      $str2 = implode("", $xyz);
      $str2 = $atoms . "\n" . $dir . "\n" . $str2;
//      drupal_set_message('String: ' . $str2);
      fwrite($fp2, $str2);
      $return_value = fclose($fp2);
    }
  }
  return $return_value;
}

/**
 * Function to unzip files and put the uncompressed files in the
 * right place
 * @global type $user
 * @param type $directory
 * @param type $do_not_add
 * @return type 
 */
function unzip_files($directory, $do_not_add) {

  global $user;

  $dir_name = "temp" . $user->uid;
  $zips = glob($directory . '/*.zip');
  $zipfile = $zips[0];
  if ($zipfile != NULL) {
    $tmp_dir = $directory . '/temp/';
    if (!is_dir($tmp_dir)) {
      mkdir($tmp_dir);
    }
    $file_name = $zipfile->filepath;
    $file_list = array();
    $cmd_string = "unzip -q -o -d " . $tmp_dir . ' ' . $zipfile;
    system($cmd_string, $ret_val);
  }

  return $zipfile;
}

/**
 * Given a chemical structure file, create a basic MODS record
 * based on the name of the file and the watch folder that it
 * was foudn in
 * @param type $file 
 */
function create_mods($file) {
  $mods = <<<MODS
  <mods xmlns="http://www.loc.gov/mods/v3" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xlink="http://www.w3.org/1999/xlink">
          <titleInfo><title>TITLE</title></titleInfo>
          <name type="personal"><namePart>NAME</namePart><role><roleTerm authority="marcrelator" type="text">Author</roleTerm></role></name>
        </mods>
MODS;

  $info = pathinfo($file);
  $title = basename($file, '.' . $info['extension']);

  $dir = dirname($file);

  $new_name = $dir . '/' . $title . '.xml';

  if (!file_exists($new_name)) {
    $path = explode('/', $file);
    $user_name = $path[(count($path) - 4)];
    $firstname_query = db_query("SELECT value FROM {profile_values} LEFT JOIN users ON profile_values.uid=users.uid LEFT JOIN profile_fields ON profile_fields.fid=profile_values.fid WHERE users.name = '%s' AND profile_fields.name = 'profile_firstname'", $user_name);
    $first_name = db_result($firstname_query);
    $lastname_query = db_query("SELECT value FROM {profile_values} LEFT JOIN users ON profile_values.uid=users.uid LEFT JOIN profile_fields ON profile_fields.fid=profile_values.fid WHERE users.name = '%s' AND profile_fields.name = 'profile_lastname'", $user_name);
    $last_name = db_result($lastname_query);
    if ($last_name == '') {
      $full_name = $user_name;
    }
    else {
      $full_name = $first_name . ' ' . $last_name;
    }
    $mods = str_replace(array('TITLE', 'NAME'), array($title, $full_name), $mods);
    $fh = fopen($new_name, 'w');
    fwrite($fh, $mods);
    fclose($fh);
  }
}

function is_empty_dir($dir) {
  return (($files = @scandir($dir)) && count($files) <= 2);
}

function check_numeric($str) {
  $ret_val = preg_match('/\d/', $str);
  return $ret_val;
}

/**
 * Replace positive and negative charges with the correct, shortened version
 * and add HTML tags to make them superscript
 * @param type $formula
 * @return type 
 */
function formula_cleanup($formula) {
  $formula2 = str_replace(' ', '', $formula);
  $formula2 = preg_replace('/([0-9])/', '<sub>$1</sub>', $formula2);

  $find = array('----', '---', '--', '++++', '+++', '++');
  $replace = array('<sup>4-</sup>', '<sup>3-</sup>', '<sup>2-</sup>', '<sup>4+</sup>', '<sup>3+</sup>', '<sup>2+</sup>', );

  $formula2 = str_replace($find, $replace, $formula2);

  return $formula2;
}

function mol_clean($mol_file) {

  $find = array("\n", "\r");
  $replace = array('\n', '');
  $file = str_replace($find, $replace, $mol_file);
  $file = 'Molecule Name\n' . $file;

  return $file;
}

function download_file($pid) {

  global $fedora_url;

  $file = $fedora_url . '/objects/' . $pid . '/datastreams/OBJ/content';

  $result = file_get_contents($file);
  $mime = file_get_mimetype($filename);
  $ext = get_file_extension($mime);
  $filename = $pid . '.' . $ext;

  drupal_add_http_header('Content-Type', $mime);
  drupal_add_http_header('Content-Disposition: attachment', 'filename=' . $filename);

  print($result) . "\r\n";

  exit;
}

function prettify($code) {
  $array = explode("\n", $code);
  $output = array();
  $needles = array("#", '**', '/*', '*/', '//');
  foreach ($array as $line) {
    if (stripos_multi($line, $needles) !== FALSE) {
      $output[] = '<b class="codecomment">' . $line . '</b>';
    }
    else {
      $output[] = $line;
    }
  }
  $out = implode("\n", $output);

  return $out;
}

function stripos_multi($haystack, $needle, $offset = '0') {
  if (!is_array($needle)) {
    $needle = array($needle);
  }//if

  foreach ($needle as $searchstring) {
    $position = stripos($haystack, $searchstring, $offset);

    if ($position !== FALSE) {
      return $position;
    }//if
  }//foreach

  return FALSE;
}

//function  

class SyntaxHighlight {

  public static function process($s) {
    $s = htmlspecialchars($s);

    // Workaround for escaped backslashes
    $s = str_replace('\\\\', '\\\\<e>', $s);

    $regexp = array(
      // Comments/Strings
      '/(
                \/\*.*?\*\/|
                \/\/.*?\n|
                \#.*?\n|
                (?<!\\\)&quot;.*?(?<!\\\)&quot;|
                (?<!\\\)\'(.*?)(?<!\\\)\'
            )/isex'
      => 'self::replaceId($tokens,\'$1\')',
      // Numbers (also look for Hex)
      '/(?<!\w)(
                0x[\da-f]+|
                \d+
            )(?!\w)/ix'
      => '<span class="N">$1</span>',
      // Make the bold assumption that an all uppercase word has a 
      // special meaning
      '/(?<!\w|>)(
                [A-Z_0-9]{2,}
            )(?!\w)/x'
      => '<span class="D">$1</span>',
      // Keywords
      '/(?<!\w|\$|\%|\@|>)(
                and|or|xor|for|do|while|foreach|as|return|die|exit|if|then|else|
                elseif|new|delete|try|throw|catch|finally|class|function|string|
                array|object|resource|var|bool|boolean|int|integer|float|double|
                real|string|array|global|const|static|public|private|protected|
                published|extends|switch|true|false|null|void|this|self|struct|
                char|signed|unsigned|short|long
            )(?!\w|=")/ix'
      => '<span class="K">$1</span>',
      // PHP/Perl-Style Vars: $var, %var, @var
      '/(?<!\w)(
                (\$|\%|\@)(\-&gt;|\w)+
            )(?!\w)/ix'
      => '<span class="V">$1</span>'
    );

    $tokens = array(); // This array will be filled from the regexp-callback
    $s = preg_replace(array_keys($regexp), array_values($regexp), $s);

    // Paste the comments and strings back in again
    $s = str_replace(array_keys($tokens), array_values($tokens), $s);

    // Delete the "Escaped Backslash Workaround Token" (TM) and replace 
    // tabs with four spaces.
    $s = str_replace(array('<e>', "\t"), array('', '    '), $s);

    return '<pre>' . $s . '</pre>';
  }

  // Regexp-Callback to replace every comment or string with a uniqid and save 
  // the matched text in an array
  // This way, strings and comments will be stripped out and wont be processed 
  // by the other expressions searching for keywords etc.
  private static function replaceId(&$a, $match) {
    $id = "##r" . uniqid() . "##";

    // String or Comment?
    if ($match{0} == '/' || $match{0} == '#') {
      $a[$id] = '<span class="C">' . $match . '</span>';
    }
    else {
      $a[$id] = '<span class="S">' . $match . '</span>';
    }
    return $id;
  }

}

function system_extension_mime_types() {
  // Returns the system MIME type mapping of extensions to MIME types, as defined in /etc/mime.types.
  $out = array();
  $file = fopen('/etc/mime.types', 'r');
  while (($line = fgets($file)) !== FALSE) {
    $line = trim(preg_replace('/#.*/', '', $line));
    if (!$line)
      continue;
    $parts = preg_split('/\s+/', $line);
    if (count($parts) == 1)
      continue;
    $type = array_shift($parts);
    foreach ($parts as $part)
      $out[$part] = $type;
  }
  fclose($file);
  return $out;
}

function system_extension_mime_type($file) {
  // Returns the system MIME type (as defined in /etc/mime.types) for the filename specified.
  //
    // $file - the filename to examine
  static $types;
  if (!isset($types))
    $types = system_extension_mime_types();
  $ext = pathinfo($file, PATHINFO_EXTENSION);
  if (!$ext)
    $ext = $file;
  $ext = strtolower($ext);
  return isset($types[$ext]) ? $types[$ext] : NULL;
}

function system_mime_type_extensions() {
  // Returns the system MIME type mapping of MIME types to extensions, as defined in /etc/mime.types (considering the first
  // extension listed to be canonical).
  $out = array();
  $file = fopen('/etc/mime.types', 'r');
  while (($line = fgets($file)) !== FALSE) {
    $line = trim(preg_replace('/#.*/', '', $line));
    if (!$line)
      continue;
    $parts = preg_split('/\s+/', $line);
    if (count($parts) == 1)
      continue;
    $type = array_shift($parts);
    if (!isset($out[$type]))
      $out[$type] = array_shift($parts);
  }
  fclose($file);
  return $out;
}

function system_mime_type_extension($type) {
  // Returns the canonical file extension for the MIME type specified, as defined in /etc/mime.types (considering the first
  // extension listed to be canonical).
  //
    // $type - the MIME type
  static $exts;
  if (!isset($exts))
    $exts = system_mime_type_extensions();
  return isset($exts[$type]) ? $exts[$type] : NULL;
}

function islandora_chem_sp_convert_spartan($file) {
  global $base_path;
  global $base_url;
  $file_array = array_filter(array_map('trim', file($file)));

  $end = array_search("ENDCART", $file_array, TRUE);

  $new_array = array();

  for ($i = 3; $i < $end; $i++) {
    $new_array[] = $file_array[$i];
  }

  $atomic_number = array(
    '23',
    '22',
    '21',
    '20',
    '19',
    '18',
    '17',
    '16',
    '15',
    '14',
    '13',
    '12',
    '11',
    '10',
    '9',
    '8',
    '7',
    '6',
    '5',
    '4',
    '3',
    '2',
    '1',
  );

  $element = array(
    'V',
    'Ti',
    'Sc',
    'Ca',
    'K',
    'Ar',
    'Cl',
    'S',
    'P',
    'Si',
    'Al',
    'Mg',
    'Na',
    'Ne',
    'F',
    'O',
    'N',
    'C',
    'B',
    'Be',
    'Li',
    'He',
    'H',
  );

  $output_array = array();

  foreach ($new_array as $line) {
    $line_array = explode('  ', $line);
    $line_array[0] = str_replace($atomic_number, $element, $line_array[0]);
    $output_array[] = implode(' ', $line_array);
  }

  $new_file = file_directory_path() . '/temp/new.xyz';

  $fh = fopen($new_file, 'w');
  fwrite($fh, implode("\n", $output_array));
  fclose($fh);

  clean_XYZ($new_file);

  return $new_file;
}

function islandora_chem_sp_convert_spartan_output($file) {

  global $base_path;
  global $base_url;
  $file_array = array_values(array_filter(array_map('trim', file($file))));
//  var_dump($file_array);  
  $coords_header = array_search("Atom            X             Y             Z", $file_array, TRUE);
  $start = ($coords_header + 2);

  $end = islandora_chem_sp_array_search("Point Group =", $file_array);

  $length = $end - $start;

  $coords_array = array_slice($file_array, $start, $length);

  $output_array = array();

  foreach ($coords_array as $coords) {
    $coords_components = explode(' ', trim($coords));
    foreach ($coords_components as $key => $component) {
      if ($component == NULL) {
        unset($coords_components[$key]);
      }
    }
    $coords_components = array_values($coords_components);
    $output_array[] = $coords_components[1] . '  ' . $coords_components[3] . '  ' . $coords_components[4] . '  ' . $coords_components[5];
  }

  $new_file = file_directory_path() . '/temp/new.xyz';

  $fh = fopen($new_file, 'w');
  fwrite($fh, implode("\n", $output_array));
  fclose($fh);

  clean_XYZ($new_file);

  return $new_file;
}

function islandora_chem_sp_url_exists($url) {
  // Version 4.x supported
  $handle = curl_init($url);
  if (FALSE === $handle) {
    return FALSE;
  }
  curl_setopt($handle, CURLOPT_HEADER, FALSE);
  curl_setopt($handle, CURLOPT_FAILONERROR, TRUE);  // this works
  curl_setopt($handle, CURLOPT_HTTPHEADER, Array("User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.15) Gecko/20080623 Firefox/2.0.0.15")); // request as if Firefox    
  curl_setopt($handle, CURLOPT_NOBODY, TRUE);
  curl_setopt($handle, CURLOPT_RETURNTRANSFER, FALSE);
  $connectable = curl_exec($handle);
  curl_close($handle);
  return $connectable;
}

function islandora_chem_sp_array_search($needle = NULL, $haystack_array = NULL, $skip = 0) {
  if ($needle == NULL || $haystack_array == NULL)
    die('$needle and $haystack_array are mandatory for functie my_array_search()');
  foreach ($haystack_array as $key => $eval) {
    if ($skip != 0) {
      $eval = substr($eval, $skip);
    }
    if (stristr($eval, $needle) !== FALSE) {
      return $key;
    }
  }
  return FALSE;
}

function islandora_chem_sp_object_profile($pid) {
  $fedora_base = variable_get(fedora_base_url , 'http://localhost:8080/fedora');
  $url = $fedora_base . '/objects/' . $pid . '?format=xml';
  var_dump($url);
  $contents = @file_get_contents($url);
  if ($contents != NULL) {
    $object = new SimpleXMLElement($contents);
  }
  else {
    return FALSE;
  }
  
  $item['owner'] = $object->objOwnerId;
  $item['label'] = $object->objLabel;
  
 return $item;
  
}

function islandora_chem_sp_get_uid($username)
{    
    // Function that returns the uid based on the username given
    $user = db_fetch_object(db_query("SELECT uid FROM users WHERE name='$username'"));

    return $user->uid;
}