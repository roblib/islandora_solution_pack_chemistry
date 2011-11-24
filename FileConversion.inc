<?php

function cleanXYZ($file) {
  $ext = strrchr($file, '.');
  $base = preg_replace("/$ext$/", '', $file);
  $ext = substr($ext, 1);
  if ($ext == 'xyz') {
    $xyz = file($file);
    drupal_set_message('First line: ' . $xyz[0]);
    if (!checkNumeric($xyz[0])) {
      drupal_set_message('Not numeric! It is ' . gettype($xyz[0]));
    }
    if (!checkNumeric($xyz[0])) {
      $fp2 = fopen($file, 'w');
      drupal_set_message('Cleaning file: ' . $file);
      $atoms = count($xyz);
      $str2 = implode("", $xyz);
      $str2 = $atoms . "\n" . $dir . "\n" . $str2;
      drupal_set_message('New xyz file:' . $str2);
      fwrite($fp2, $str2);
      fclose($fp2);
    }
  }
}

function unzipFiles($directory, $doNotAdd) {

  global $user;

  $dirName = "temp" . $user->uid;
  $zips = glob($directory . '/*.zip');
  $zipfile = $zips[0];
  if ($zipfile != NULL) {
//  $tmpDir = file_directory_path() . '/' . $dirName . '/';
    $tmpDir = $directory . '/temp/';
    if (!is_dir($tmpDir)) {
      mkdir($tmpDir);
    }
    $fileName = $zipfile->filepath;
    $file_list = array();
    $cmdString = "unzip -q -o -d " . $tmpDir . ' ' . $zipfile;
    system($cmdString, $retVal);
  }

  return $zipfile;
}

function createMODS($file) {
  $mods = <<<MODS
  <mods xmlns="http://www.loc.gov/mods/v3" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xlink="http://www.w3.org/1999/xlink">
          <titleInfo><title>TITLE</title></titleInfo>
          <name type="personal"><namePart>NAME</namePart><role><roleTerm authority="marcrelator" type="text">Author</roleTerm></role></name>
        </mods>
MODS;

  $info = pathinfo($file);
  $title = basename($file, '.' . $info['extension']);

  $dir = dirname($file);

  $newName = $dir . '/' . $title . '.xml';

  if (!file_exists($newName)) {
    $path = explode('/', $file);
    drupal_set_message('Name path: ' . $path[5]);
    $name = $path[5];

    $mods = str_replace(array('TITLE', 'NAME'), array($title, $name), $mods);

    drupal_set_message($mods);

    drupal_set_message('MODS name: ' . $newName);
    $fh = fopen($newName, 'w');
    fwrite($fh, $mods);
    fclose($fh);
  }
}

function isEmptyDir($dir) {
  return (($files = @scandir($dir)) && count($files) <= 2);
}

function checkNumeric($str) {
  $retVal = preg_match('/\d/', $str);
  return $retVal;
}

function formulaCleanup($formula) {
  $formula2 = str_replace(' ', '', $formula);
  $formula2 = preg_replace('/([0-9])/', '<sub>$1</sub>', $formula2);

  $find = array('----', '---', '--', '++++', '+++', '++');
  $replace = array('<sup>4-</sup>', '<sup>3-</sup>', '<sup>2-</sup>', '<sup>4+</sup>', '<sup>3+</sup>', '<sup>2+</sup>',);

  $formula2 = str_replace($find, $replace, $formula2);

//      $formula2 = str_replace('----', '<sup>4-</sup>', $formula2);
//      $formula2 = str_replace('---', '<sup>3-</sup>', $formula2);
//      $formula2 = str_replace('--', '<sup>2-</sup>', $formula2);
//      $formula2 = str_replace('++++', '<sup>4+</sup>', $formula2);
//      $formula2 = str_replace('+++', '<sup>3+</sup>', $formula2);
//      $formula2 = str_replace('++', '<sup>2+</sup>', $formula2);

  return $formula2;
}

function molClean($molFile) {

  $find = array("\n", "\r");
  $replace = array('\n', '');
  $file = str_replace($find, $replace, $molFile);
  $file = 'Molecule Name\n' . $file;

  return $file;
}

function downloadFile($pid) {
  
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

function stripos_multi ($haystack, $needle, $offset = '0') {
 if (!is_array($needle)) {
  $needle = array($needle);
 }//if

 foreach ($needle as $searchstring) {
  $position = stripos($haystack, $searchstring, $offset);

  if ($position !== false) {
   return $position;
  }//if
 }//foreach

 return false;
}//function  

class SyntaxHighlight {
    public static function process( $s ) {
        $s = htmlspecialchars( $s );
        
        // Workaround for escaped backslashes
        $s = str_replace( '\\\\','\\\\<e>', $s ); 
        
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
        
        return '<pre>'.$s.'</pre>';
    }
    
    // Regexp-Callback to replace every comment or string with a uniqid and save 
    // the matched text in an array
    // This way, strings and comments will be stripped out and wont be processed 
    // by the other expressions searching for keywords etc.
    private static function replaceId( &$a, $match ) {
        $id = "##r".uniqid()."##";
        
        // String or Comment?
        if( $match{0} == '/' || $match{0} == '#' ) {
            $a[$id] = '<span class="C">'.$match.'</span>';
        } else {
            $a[$id] = '<span class="S">'.$match.'</span>';
        }
        return $id;
    }
}

function system_extension_mime_types() {
    # Returns the system MIME type mapping of extensions to MIME types, as defined in /etc/mime.types.
    $out = array();
    $file = fopen('/etc/mime.types', 'r');
    while(($line = fgets($file)) !== false) {
        $line = trim(preg_replace('/#.*/', '', $line));
        if(!$line)
            continue;
        $parts = preg_split('/\s+/', $line);
        if(count($parts) == 1)
            continue;
        $type = array_shift($parts);
        foreach($parts as $part)
            $out[$part] = $type;
    }
    fclose($file);
    return $out;
}

function system_extension_mime_type($file) {
    # Returns the system MIME type (as defined in /etc/mime.types) for the filename specified.
    #
    # $file - the filename to examine
    static $types;
    if(!isset($types))
        $types = system_extension_mime_types();
    $ext = pathinfo($file, PATHINFO_EXTENSION);
    if(!$ext)
        $ext = $file;
    $ext = strtolower($ext);
    return isset($types[$ext]) ? $types[$ext] : null;
}

function system_mime_type_extensions() {
    # Returns the system MIME type mapping of MIME types to extensions, as defined in /etc/mime.types (considering the first
    # extension listed to be canonical).
    $out = array();
    $file = fopen('/etc/mime.types', 'r');
    while(($line = fgets($file)) !== false) {
        $line = trim(preg_replace('/#.*/', '', $line));
        if(!$line)
            continue;
        $parts = preg_split('/\s+/', $line);
        if(count($parts) == 1)
            continue;
        $type = array_shift($parts);
        if(!isset($out[$type]))
            $out[$type] = array_shift($parts);
    }
    fclose($file);
    return $out;
}

function system_mime_type_extension($type) {
    # Returns the canonical file extension for the MIME type specified, as defined in /etc/mime.types (considering the first
    # extension listed to be canonical).
    #
    # $type - the MIME type
    static $exts;
    if(!isset($exts))
        $exts = system_mime_type_extensions();
    return isset($exts[$type]) ? $exts[$type] : null;
}