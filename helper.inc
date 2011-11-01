<?php

function bchexdec($hex) {
        if (strlen($hex) == 1) {
            return hexdec($hex);
        } 
        else {
            $remain = substr($hex, 0, -1);
            $last = substr($hex, -1);
            return bcadd(bcmul(16, bchexdec($remain)), hexdec($last));
        }
    }
    
    function fingerprintdec($fingerprint) {
      
      $fingerprint = explode(' ', implode(' ', $fingerprint));
      $fp1 = $fingerprint[0] . $fingerprint[1];
      $fp1d = bchexdec($fp1);
      $fp2 = $fingerprint[2] . $fingerprint[3];
      $fp2d = bchexdec($fp2);
      $fp3 = $fingerprint[4] . $fingerprint[5];
      $fp3d = bchexdec($fp3);
      $fp4 = $fingerprint[6] . $fingerprint[7];
      $fp4d = bchexdec($fp4);
      $fp5 = $fingerprint[8] . $fingerprint[9];
      $fp5d = bchexdec($fp5);
      $fp6 = $fingerprint[10] . $fingerprint[11];
      $fp6d = bchexdec($fp6);
      $fp7 = $fingerprint[12] . $fingerprint[13];
      $fp7d = bchexdec($fp7);
      $fp8 = $fingerprint[14] . $fingerprint[15];
      $fp8d = bchexdec($fp8);
      $fp9 = $fingerprint[16] . $fingerprint[17];
      $fp9d = bchexdec($fp9);
      $fp10 = $fingerprint[18] . $fingerprint[19];
      $fp10d = bchexdec($fp10);
      $fp11 = $fingerprint[20] . $fingerprint[21];
      $fp11d = bchexdec($fp11);
      $fp12 = $fingerprint[22] . $fingerprint[23];
      $fp12d = bchexdec($fp12);
      $fp13 = $fingerprint[24] . $fingerprint[25];
      $fp13d = bchexdec($fp13);
      $fp14 = $fingerprint[26] . $fingerprint[27];
      $fp14d = bchexdec($fp14);
      $fp15 = $fingerprint[28] . $fingerprint[29];
      $fp15d = bchexdec($fp15);
      $fp16 = $fingerprint[30] . $fingerprint[31];
      $fp16d = bchexdec($fp16);
      
      return array($fp1d, $fp2d, $fp3d, $fp4d, $fp5d, $fp6d, $fp7d, $fp8d, $fp9d, $fp10d, $fp11d, $fp12d, $fp13d, $fp14d, $fp15d, $fp16d);
      
    }