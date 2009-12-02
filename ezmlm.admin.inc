<?php
// $Id$
// $Name$
// for drupal 7.x

/**
 * @file
 * administration functions for ezmlm
 */

/**
 * Menu callback; presents the settings form for ezmlm
 */
function ezmlm_admin_settings() {
  // add mailing list
  $form['ezmlm_add'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add mailing lists'),
    '#description' => t('Add new lists'),
    '#collapsible' => TRUE,
  );
  $form['ezmlm_add']['ezmlm_add_name'] = array(
    '#type' => 'textfield',
    '#title' => t('New list name'),
    '#size' => 20,
    '#maxlength' => 20,
    '#required' => FALSE,
  );
  $form['ezmlm_add']['ezmlm_add_address'] = array(
    '#type' => 'textfield',
    '#title' => t('New list address'),
    '#size' => 30,
    '#maxlength' => 50,
    '#required' => FALSE,
  );

  // Delete mailing lists
  if (_ezmlm_get_count() > 0) {
    $form['ezmlm_delete'] = array(
      '#type' => 'fieldset',
      '#title' => t('Remove mailing lists'),
      '#description' => t('Check the lists you want to remove'),
      '#collapsible' => TRUE,
    );

    $lists = variable_get('ezmlmmailinglists', '');
    if (is_array($lists)) {
      foreach ($lists as $list_text => $list_email) {
        $list_text2 = preg_replace('/\s/', '~', $list_text);
        $form['ezmlm_delete'][$list_text2] = array(
          '#type' => 'checkbox',
          '#title' => "$list_text ($list_email)" ,
        );
      }
    }

    // registration
    $form['ezmlm_reg'] = array(
      '#type' => 'fieldset',
      '#title' => t('Registration settings'),
      '#description' => t('Allow new users to select mailing lists on registration'),
      '#collapsible' => TRUE,
    );
    $form['ezmlm_reg']['ezmlm_register'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display lists in registration form'),
      '#default_value' => variable_get('ezmlm_register', 1),
    );
    if (is_array($lists)) {
      $reglist = variable_get('ezmlm_register_list', '');
      foreach ($lists as $list_text => $list_email) {
        $list_text2 = preg_replace('/\s/', '~', $list_text);
        $form['ezmlm_reg']["reg_$list_text2"] = array(
          '#type' => 'checkbox',
          '#title' => "$list_text ($list_email)" ,
          '#default_value' => ((is_array($reglist) && in_array($list_email, $reglist)) ? 1 : 0),
        );
      }
    }
    $form['ezmlm_reg']['ezmlm_reg_display'] = array(
      '#type' => 'radios',
      '#title' => t('List display type'),
      '#default_value' => variable_get('ezmlm_reg_display', 'email'),
      '#options' => array('email' => 'List address', 'name' => 'List name'),
      '#description' => t('Display %listaddress or %listname on the registration page', array('%listaddress' => 'list address', '%listname' => 'list name')),
    );
  }

  $form['ezmlm_misc'] = array(
      '#type' => 'fieldset',
      '#title' => t('Various settings'),
      '#collapsible' => TRUE,
  );
  $form['ezmlm_misc']['ezmlm_title'] = array(
    '#type' => 'textfield',
    '#title' => t('List title'),
    '#description' => t('Configure the title of the subscription list'),
    '#default_value' => variable_get('ezmlm_title', t('Mailing lists')),
    '#size' => 20,
    '#maxlength' => 20,
    '#required' => FALSE,
  );
  $form['ezmlm_misc']['ezmlm_display'] = array(
    '#type' => 'radios',
    '#title' => t('List display type'),
    '#default_value' => variable_get('ezmlm_display', 'email'),
    '#options' => array('email' => 'List address', 'name' => 'List name'),
    '#description' => t('Display %listaddress or %listname on the subscription page', array('%listaddress' => 'list address', '%listname' => 'list name')),
  );
  $form['ezmlm_misc']['ezmlm_block_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Block title'),
    '#description' => t('Configure the title of the block subscription list'),
    '#default_value' => variable_get('ezmlm_block_title', t('Subscriptions')),
    '#size' => 20,
    '#maxlength' => 20,
    '#required' => FALSE,
  );
  $form['ezmlm_misc']['ezmlm_block_display'] = array(
    '#type' => 'radios',
    '#title' => t('Block display type'),
    '#default_value' => variable_get('ezmlm_block_display', 'email'),
    '#options' => array('email' => 'List address', 'name' => 'List name'),
    '#description' => t('Display %listaddress or %listname on the block subscription page', array('%listaddress' => 'list address', '%listname' => 'list name')),
  );
  $form['#validate'][] = 'ezmlm_admin_settings_validate';
  $form['#submit'][] = 'ezmlm_admin_settings_submit';

  return system_settings_form($form);
}

/**
 * Validate form
 */
function ezmlm_admin_settings_validate($form, &$form_state) {

  if (drupal_strlen($form_state['values']['ezmlm_add_address'])) {
    if ($error = user_validate_mail($form_state['values']['ezmlm_add_address'])) {
      form_set_error('ezmlm_add_address', $error);
    }
    if (! drupal_strlen($form_state['values']['ezmlm_add_name'])) {
      form_set_error('ezmlm_add_name', t('You must fill in the Name field'));
    }
  }
}

/**
 * Submit form
 */
function ezmlm_admin_settings_submit($form, &$form_state) {
  // various
  if (isset($form_state['values']['ezmlm_title']) && $form_state['values']['ezmlm_title'] != "") {
    variable_set('ezmlm_title', $form_state['values']['ezmlm_title']);
  }
  if (isset($form_state['values']['ezmlm_block_title']) && $form_state['values']['ezmlm_block_title'] != "") {
    variable_set('ezmlm_block_title', $form_state['values']['ezmlm_block_title']);
  }
  // look for deletes
  $lists = _ezmlm_get_lists();
  if (is_array($lists)) {
    // deletions
    $newlists = array();
    $delct = 0;
    foreach ($lists as $list_text => $list_email) {
      $list_text = preg_replace('/\s/', '~', $list_text);
      if (isset($form_state['values'][$list_text]) && $form_state['values'][$list_text] == 1) {
        $delct++;
        continue;     // skip copying this list to the new list
      }
      else {
        $list_text = preg_replace('/~/', ' ', $list_text);
        $newlists[$list_text] = $list_email;   // copy old list entry to new
      }
    }
    if (count($newlists) < count($lists)) {
      variable_set('ezmlmmailinglists', $newlists);
      drupal_set_message(t('Deletions done.'));
    }
  }
  // add list
  $lists = _ezmlm_get_lists();
  $count = _ezmlm_get_count();
  if (!is_array($lists) || $count < 1 ) {
    $lists = array();
    variable_del('ezmlmmailinglists');
  }
  $list_text = trim($form_state['values']['ezmlm_add_name']);
  $list_email = trim($form_state['values']['ezmlm_add_address']);
  if (drupal_strlen($list_text) && drupal_strlen($list_email) ) {
    $lists[$list_text] = $list_email;
    variable_set('ezmlmmailinglists', $lists);
    drupal_set_message( t('The mailing lists have been saved.'));
  }
  // registration
  if ( isset($form_state['values']['ezmlm_register'])) {
    variable_set('ezmlm_register', 1);
  }
  else {
    variable_set('ezmlm_register', 0);
  }

  if ( variable_get('ezmlm_register', 0) ) {
    // reg list
    if (is_array($lists) && $count > 0) {
      variable_del('ezmlm_register_list');
      $arr = array();
      foreach ($lists as $list_text => $list_email) {
        $list_text = preg_replace('/\s/', '~', $list_text);
        if ( isset($form_state['values']["reg_$list_text"]) && $form_state['values']["reg_$list_text"] == 1 ) {
          $list_text = preg_replace('/~/', ' ', $list_text);
          $arr[$list_text] = $list_email;
        }
      }
      if ( count($arr) ) {
        variable_set('ezmlm_register_list', $arr);
      }
    }
  }
  // display
  if ( isset($form_state['values']['ezmlm_display'])) {
    variable_set('ezmlm_display', $form_state['values']['ezmlm_display']);
  }
  if (isset($form_state['values']['ezmlm_block_display'])) {
    variable_set('ezmlm_block_display', $form_state['values']['ezmlm_block_display']);
  }
  if ( isset($form_state['values']['ezmlm_reg_display'])) {
    variable_set('ezmlm_reg_display', $form_state['values']['ezmlm_reg_display']);
  }
}
