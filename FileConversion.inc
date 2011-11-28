<?php

/**
 * @file
 */

function clean_XYZ($file) {
  $ext = strrchr($file, '.');
  $base = preg_replace("/$ext$/", '', $file);
  $ext = substr($ext, 1);
  if ($ext == 'xyz') {
    $xyz = file($file);
    if (!checkNumeric($xyz[0])) {
    }
    if (!checkNumeric($xyz[0])) {
      $fp2 = fopen($file, 'w');
      $atoms = count($xyz);
      $str2 = implode("", $xyz);
      $str2 = $atoms . "\n" . $dir . "\n" . $str2;
      fwrite($fp2, $str2);
      fclose($fp2);
    }
  }
}

function unzip_files($directory, $do_not_add) {

  global $user;

  $dir_name = "temp" . $user->uid;
  $zips = glob($directory . '/*.zip');
  $zipfile = $zips[0];
  if ($zipfile != NULL) {
//  $tmpDir = file_directory_path() . '/' . $dirName . '/';
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
    $name = $path[5];
    $mods = str_replace(array('TITLE', 'NAME'), array($title, $name), $mods);
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

function formula_cleanup($formula) {
  $formula2 = str_replace(' ', '', $formula);
  $formula2 = preg_replace('/([0-9])/', '<sub>$1</sub>', $formula2);

  $find = array('----', '---', '--', '++++', '+++', '++');
  $replace = array('<sup>4-</sup>', '<sup>3-</sup>', '<sup>2-</sup>', '<sup>4+</sup>', '<sup>3+</sup>', '<sup>2+</sup>', );

  $formula2 = str_replace($find, $replace, $formula2);

//      $formula2 = str_replace('----', '<sup>4-</sup>', $formula2);
//      $formula2 = str_replace('---', '<sup>3-</sup>', $formula2);
//      $formula2 = str_replace('--', '<sup>2-</sup>', $formula2);
//      $formula2 = str_replace('++++', '<sup>4+</sup>', $formula2);
//      $formula2 = str_replace('+++', '<sup>3+</sup>', $formula2);
//      $formula2 = str_replace('++', '<sup>2+</sup>', $formula2);

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
}//function  

class SyntaxHighlight {
    public static function process( $s ) {
        $s = htmlspecialchars( $s );
        
        // Workaround for escaped backslashes
        $s = str_replace( '\\\\', '\\\\<e>', $s ); 
        
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
        $s = preg_replace( array_keys($regexp), array_values($regexp), $s );
        
        // Paste the comments and strings back in again
        $s = str_replace( array_keys($tokens), array_values($tokens), $s );
        
        // Delete the "Escaped Backslash Workaround Token" (TM) and replace 
        // tabs with four spaces.
        $s = str_replace( array( '<e>', "\t" ), array( '', '    ' ), $s );
        
        return '<pre>' . $s . '</pre>';
    }
    
    // Regexp-Callback to replace every comment or string with a uniqid and save 
    // the matched text in an array
    // This way, strings and comments will be stripped out and wont be processed 
    // by the other expressions searching for keywords etc.
    private static function replaceId( &$a, $match ) {
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

/**
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
 * Autocomplete the content model name..
 * 
 * @param string $string
 * @return string
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
 * Autocomplete the content model name..
 * 
 * @param string $string
 * @return string
 */
function islandora_content_model_forms_autocomplete_institutions($string = '') {
  $content_models = islandora_chem_sp_get_values('rels.hasModel:researcher_CM', 'mads.organization');
  // @todo find an alternative solution to array filter because of earlier versions 5.2
  $output = array_filter($content_models, new AutoCompleteFilterChem($string));
  $output[$string] = $string;
  return drupal_json($output);
}

/**
 *
 * @param string $content_model
 * @return array 
 */
function islandora_chem_sp_get_values($query, $results) {
  
  $query = file_get_contents('http://192.168.200.128:8080/solr/select?q=' . $query . '&fl=' . $results);
  
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