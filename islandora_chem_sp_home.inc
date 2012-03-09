<?php

/**
 * @file
 */

/**
 * Form to display content on the user's home page
 * @global type $user
 * @param type $form_state
 * @return type 
 */
function islandora_chem_sp_user_login_form(&$form_state) {
  global $user;
  $form = array();

  $form['welcome'] = array(
    '#value' => t('Welcome @user', array('@user' => $user->name)),
  );

  $form['first_tab'] = array(
    '#type' => 'tabpage',
  );

  $form['first_tab']['tabs'] = array(
    '#type' => 'tabset',
  );

  $form['first_tab']['tabs']['files'] = array(
    '#type' => 'tabpage',
    '#title' => t('Files'),
  );

  $form['first_tab']['tabs']['files']['content'] = array(
    '#value' => drupal_get_form('islandora_chem_sp_object_form'),
  );

  $form['first_tab']['tabs']['files']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#name' => 'save_permissions',
  );

  $form['first_tab']['tabs']['conversations'] = array(
    '#type' => 'tabpage',
    '#title' => t('Conversations'),
  );

  $form['first_tab']['tabs']['conversations']['tabs'] = array(
    '#type' => 'tabset',
  );

  $form['first_tab']['tabs']['conversations']['tabs']['new_groups'] = array(
    '#type' => 'tabpage',
    '#title' => t('Group content')
  );

  $form['first_tab']['tabs']['conversations']['tabs']['new_groups']['content'] = array(
    '#value' => views_embed_view('og_mytracker'),
  );

  $form['first_tab']['tabs']['conversations']['tabs']['unread_groups'] = array(
    '#type' => 'tabpage',
    '#title' => t('Unread group content')
  );

  $form['first_tab']['tabs']['conversations']['tabs']['unread_groups']['content'] = array(
    '#value' => views_embed_view('og_unread'),
  );

  $form['first_tab']['tabs']['conversations']['tabs']['new_content'] = array(
    '#type' => 'tabpage',
    '#title' => t('Recent site content')
  );

  $form['first_tab']['tabs']['conversations']['tabs']['new_content']['content'] = array(
    '#value' => views_embed_view('og_tracker'),
  );

  $form['first_tab']['tabs']['people'] = array(
    '#type' => 'tabpage',
    '#title' => t('People')
  );

  $form['first_tab']['tabs']['people']['content'] = array(
    '#value' => drupal_get_form('islandora_chem_sp_people_form'),
  );

  $form['first_tab']['tabs']['search'] = array(
    '#type' => 'tabpage',
    '#title' => t('Search')
  );

  $form['first_tab']['tabs']['search']['simple_search'] = array(
    '#type' => 'textfield',
    '#prefix' => '<div class="mw_range_submit">',
    '#suffix' => '</div>',
  );

  $form['first_tab']['tabs']['search']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
    '#name' => 'simple_submit',
    '#prefix' => '<div class="mw_range_submit">',
    '#suffix' => '</div>',
  );

  return $form;
}

/**
 * Submit function for home page form
 * Checks which submit button was clicked and acts appropriately
 * @todo When the return key is pressed work out which button the user wanted to click
 * from elements that the user had filled in
 * @global type $user
 * @param type $form
 * @param type $form_state 
 */
function islandora_chem_sp_user_login_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_chem_sp', 'islandora_chem_sp_search');
  module_load_include('module', 'islandora_xacml_editor', 'islandora_xacml_editor');
  module_load_include('inc', 'islandora_xacml_editor', 'api/Xacml');

  global $user;
  $dsid = 'POLICY';
  $permissions = array();

  $pid_list = explode(' ', $form_state['clicked_button']['#post']['pids']);
  $open_read_pids = explode(' ', $form_state['clicked_button']['#post']['open_read_pids']);
  $open_write_pids = explode(' ', $form_state['clicked_button']['#post']['open_write_pids']);

//  krumo($open_read_pids);
//  krumo($open_write_pids);
  if ($form_state['clicked_button']['#name'] == 'simple_submit') {
    islandora_chem_sp_simple_search_form_submit($form, $form_state);
  }

  if ($form_state['clicked_button']['#name'] == 'save_permissions') {
    foreach ($form_state['clicked_button']['#post'] as $key => $value) {
      if (strstr($key, 'chem')) {
        $tasks = explode('__', $key);
        $permissions[$tasks[1]][$tasks[0]] = $value;
      }
    }
  }
//  krumo($form_state);
  foreach ($pid_list as $pid) {
    $read_open = TRUE;
    $write_open = TRUE;
    $xacml = new Xacml();
    $object = new Fedora_Item($pid);
    $datastreams = $object->get_datastreams_list_as_array();
//    drupal_set_message('PID: ' . $pid);

    foreach ($permissions[$pid] as $key => $pid_permission) {
//      drupal_set_message('Pid permission: ' . $key . ' - ' . $pid_permission);

      if ($key == 'chem_user_read' && $pid_permission == 1) {
//        drupal_set_message('Chem user read');
        $xacml->viewingRule->addUser($user->name);
        $read_open = FALSE;
      }

      if ($key == 'chem_user_write' && $pid_permission == 1) {
//        drupal_set_message('Chem user write');
        $xacml->managementRule->addUser($user->name);
        $write_open = FALSE;
      }

      if (stristr($key, 'chem_group') != FALSE) {
        $key_array = explode('-', $key);
//        drupal_set_message('Group id: ' . $key_array[1]);
//        drupal_set_message('Level: ' . $key_array[2]);
        $query = db_query("SELECT title FROM {node} WHERE nid = '%d'", $key_array[1]);
        $result = db_fetch_object($query);
        $role = $result->title;
        if ($key_array[2] == 'read') {
//          drupal_set_message('Adding viewing role ' . $role);
          $xacml->viewingRule->addRole($role);
          $read_open = FALSE;
        }
        if ($key_array[2] == 'write') {
//          drupal_set_message('Adding management role ' . $role);
          $xacml->viewingRule->addRole($role);
          $xacml->managementRule->addRole($role);
          $write_open = FALSE;
        }
      }
    }
//    drupal_set_message('Open read pids: ' . implode(', ', $open_read_pids));
//    drupal_set_message('In read: ' . (string) in_array($pid, $open_read_pids));

    if ($key == 'chem_all_read' && $pid_permission == 1) {
      if ($read_open == TRUE || !in_array($pid, $open_read_pids)) {
//        drupal_set_message('Chem all read');
        $read_roles = $xacml->viewingRule->getRoles();
        foreach ($read_roles as $read_role) {
          $xacml->viewingRule->removeRole($read_role);
        }
        $read_users = $xacml->viewingRule->getUsers();
        foreach ($read_users as $read_user) {
          $xacml->viewingRule->removeUser($read_user);
        }
      }
    }

    if ($key == 'chem_all_write' && $pid_permission == 1) {
      if ($write_open == TRUE || !in_array($pid, $open_write_pids)) {
//        drupal_set_message('Chem all write');
        $write_roles = $xacml->managementRule->getRoles();
        foreach ($write_roles as $write_role) {
          $xacml->managementRule->removeRole($write_role);
        }
        $write_users = $xacml->managementRule->getUsers();
        foreach ($write_users as $write_user) {
          $xacml->managementRule->removeUser($write_user);
        }
      }
    }


    $xml = $xacml->getXmlString();
//    krumo($xml);
    if ($xml != NULL) {

//      krumo($xml);

      if (array_key_exists($dsid, $datastreams)) {
        $object->modify_datastream_by_value($xml, $dsid, 'Xacml Policy Stream', 'text/xml');
      }
      else {
        $object->add_datastream_from_string($xml, $dsid, 'Xacml Policy Stream', 'text/xml', 'X');
      }
    }
    else {
      $result = $object->purge_datastream($dsid);
      if ($result) {
        drupal_set_message(t('Security policy removed from @pid', array('@pid' => $pid)));
      }
    }
//    drupal_set_message('XACML: ' . $xml);
  }
}

/**
 * Form to display the objects that the user owns
 * Also allows the permissions to be changed on the object by
 * editing the XACML policy
 * @todo Possibly add in individual users to assign permissions to
 * @global type $base_url
 * @global type $user
 * @param type $form_state
 * @param type $username
 * @return string 
 */
function islandora_chem_sp_object_form(&$form_state) {
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'islandora_xacml_editor', 'api/Xacml');

  global $base_url;
  global $user;

  $username = $user->name;
  $xacml_dsid = 'POLICY';
  $groups = islandora_chem_sp_og_users_groups();
  $page = isset($_GET['page']) ? $_GET['page'] : '0';
  $items_per_page = 5;
  $offset = $page * $items_per_page;

  $calculations_list = array();
  $solr_url = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');
  $solr_query = 'http://' . $solr_url . '/select?q=fgs.ownerId:"' . urlencode($username) . '"%20AND%20rel.hasModel:info\:fedora/quantumchem-sys\:sp_chem_CM&fl=PID,dc.title,cml.name&qt=standard&start=' . $offset . '&rows=' . $items_per_page;
  $solr_result = @file_get_contents($solr_query);
  if ($solr_result != NULL) {
    $calc_search = new SimpleXMLElement($solr_result);
    $pid_results = $calc_search->result;
    $pid_list = array();
    $rows2 = array();
    $open_read_pids = array();
    $open_write_pids = array();
    $group_headers = array();
    $total = $calc_search->result['numFound'];

    foreach ($pid_results->children() as $child) {
      $pid = (string) $child->str;
      $pid_list[] = $pid;
      $object = new Fedora_Item($pid);
      $datastreams = $object->get_datastreams_list_as_array();

      if (array_key_exists('POLICY', $datastreams)) {
        module_load_include('inc', 'islandora_xacml_api', 'XacmlException');

        // here we populate the form
        $xml = $object->get_datastream_dissemination($xacml_dsid);

        // some basic error handling
        try {
          $xacml = new Xacml($xml);
        } catch (XacmlException $e) {
          drupal_set_message($e->getMessage());
          drupal_set_message("Xacml Parser failed to parse $object_pid/$xacml_dsid. It is likely this POLICY wasn't written by the islandora XACML editor, it will have to be modified by hand.");
          drupal_not_found();
          exit();
        }
      }
      else {
        $xacml = new Xacml();
      }

      $read_roles = $xacml->viewingRule->getRoles();
      $write_roles = $xacml->managementRule->getUsers();

//    var_dump($read_roles);
//    var_dump($write_roles);

      $cml = $object->get_datastream_dissemination('CML');
      if ($cml != NULL) {
        $cml_xml = new SimpleXMLElement($cml);
        $name = $cml_xml->xpath('//molecule/name');
      }
      else {
        $name = array('Error retrieving name');
      }

      $dc = $object->get_datastream_dissemination('DC');
      if ($dc != NULL) {
        $dc_xml = new SimpleXMLElement($dc);
        $dc_xml->registerXPathNamespace('dc', 'http://purl.org/dc/elements/1.1/');
        $title = $dc_xml->xpath('//dc:title');
        if ($title == NULL) {
          $title = array('Unknown');
        }
      }
      else {
        $title = array('Error retrieving title');
      }
      $calc_title = l((string) $title[0] . ' ', $base_url . '/fedora/repository/' . $pid);
      $calc_name = (string) $name[0];
//    if (strlen($calc_name) > 100) {
//      $calc_name = substr($calc_name, 0, 100);
//    }
      $tick_box1 = array(
        '#type' => 'checkbox',
        '#name' => 'chem_user_read__' . $pid,
        '#value' => $xacml->viewingRule->isPopulated(),
      );
      $tick_box2 = array(
        '#type' => 'checkbox',
        '#name' => 'chem_user_write__' . $pid,
        '#value' => $xacml->managementRule->isPopulated(),
      );

      $group_checkboxes = array();
      $group_headers = array();

      foreach ($groups as $group) {

        $group_headers[] = $group['title'];

        $tick_box3 = $tick_box_group_read_ . $group['nid'];
        $tick_box3 = array(
          '#type' => 'checkbox',
          '#name' => 'chem_group-' . $group['nid'] . '-read__' . $pid,
          '#value' => in_array($group['title'], $xacml->viewingRule->getRoles()),
        );
        $group_checkboxes_string = drupal_render($tick_box3);

        $tick_box4 = $tick_box_group_write_ . $group['nid'];
        $tick_box4 = array(
          '#type' => 'checkbox',
          '#name' => 'chem_group-' . $group['nid'] . '-write__' . $pid,
          '#value' => in_array($group['title'], $xacml->managementRule->getRoles()),
        );
        $group_checkboxes_string .= drupal_render($tick_box4);
        $group_checkboxes[] = $group_checkboxes_string;
      }

      $tick_box5 = array(
        '#type' => 'checkbox',
        '#name' => 'chem_all_read__' . $pid,
        '#title' => t('Read'),
        '#value' => !$xacml->viewingRule->isPopulated(),
      );
      $tick_box6 = array(
        '#type' => 'checkbox',
        '#name' => 'chem_all_write__' . $pid,
        '#title' => t('Write'),
        '#value' => !$xacml->managementRule->isPopulated(),
      );

      if (!$xacml->viewingRule->isPopulated()) {
        $open_read_pids[] = $pid;
      }

      if (!$xacml->managementRule->isPopulated()) {
        $open_write_pids[] = $pid;
      }

      $rows2_1 = array($calc_title, ucfirst(ltrim($calc_name)), drupal_render($tick_box1) . drupal_render($tick_box2));
      $rows2_3 = array(drupal_render($tick_box5) . drupal_render($tick_box6));

      $rows2[] = array_merge($rows2_1, $group_checkboxes, $rows2_3);
    }

    $no_of_items = count($rows2);
    $element = 0;
    $GLOBALS['pager_page_array'][$element] = max(0, min((int) $pager_page_array[$element], ((int) $pager_total[$element]) - 1));

    if ($total == 0) {
      $GLOBALS['pager_total'][$element] = 0;
    }
    else {
      $GLOBALS['pager_total'][$element] = ceil($no_of_items / $total);
    }

    $header2_1 = array('Title', 'Name', 'User');
    $header_2_3 = array('All');

    $header2 = array_merge($header2_1, $group_headers, $header_2_3);

//  var_dump($header2);

    $content1 = "</BR><B>Files</B></BR>";
    if (!empty($rows2)) {
      sort($rows2);
      $content1 .= theme('table', $header2, $rows2);
    }
    else {
      $content1 .= "</BR>None found";
    }

    //Pretend we're actually doing an SQL query so paging will work
    pager_query("SELECT %d", $items_per_page, 0, NULL, $total);

    $output = $content1 . theme('pager');
  }
  else {
    $output = "</BR>None found";
  }
  $form['content'] = array(
    '#value' => $output,
  );

  $form['pids'] = array(
    '#type' => 'hidden',
    '#value' => implode(' ', $pid_list),
  );

  $form['open_read_pids'] = array(
    '#type' => 'hidden',
    '#value' => implode(' ', $open_read_pids),
  );

  $form['open_write_pids'] = array(
    '#type' => 'hidden',
    '#value' => implode(' ', $open_write_pids),
  );

  return $form;
}

/**
 * Helper function to bring back the organic groups that
 * the user is a member of. Strangely, OG doesn't provide this in their
 * API
 * @global type $user 
 * @return type array of organic groups that the user is a member of
 */
function islandora_chem_sp_og_users_groups() {
  global $user;
  $rows = array();
  $query = db_query("SELECT a.nid,title FROM {node} a INNER JOIN {og_uid} b ON a.nid = b.nid WHERE b.uid = '%s'", $user->uid);
  if ($query == FALSE) {
    return FALSE;
  }
  else {
    while ($result = db_fetch_array($query)) {
      $rows[] = $result;
    }
    return $rows;
  }
}

function islandora_chem_sp_people_form(&$form_state) {

  global $user;

  $content = '';

  $interests_query = db_query("SELECT v.value FROM {profile_values} v INNER JOIN {profile_fields} f ON v.fid=f.fid WHERE v.uid = '%d' AND f.name = 'profile_interests'", $user->uid);

  while ($result = db_fetch_array($interests_query)) {
    $interests_array[] = $result;
  }

  $interests_array = (explode("\n", $interests_array[0]['value']));
  $interesting_people = array();

  foreach ($interests_array as $interest_array) {
    $query = db_query("SELECT v.uid FROM {profile_values} v INNER JOIN {profile_fields} f ON v.fid=f.fid WHERE v.value LIKE '%%%s%%'", trim($interest_array));
    while ($result = db_fetch_array($query)) {
      $users_array[] = $result;
    }
    foreach ($users_array as $user_array) {
      $user_query = db_query("SELECT users.name FROM {users} where uid = '%d'", $user_array);
      while ($result = db_fetch_array($user_query)) {
        $interesting_people[$user_array['uid']] = $result['name'];
      }
    }
  }
  $content .= '<h3>People with similar interests</h3>';

  $content .= '<ul>';

  foreach ($interesting_people as $uid => $person) {
    $content .= '<li>' . l($person, 'user/' . $uid) . '</li>';
  }

  $content .= '</ul>';

  $form['people_list'] = array(
    '#value' => $content,
  );

  return $form;
}