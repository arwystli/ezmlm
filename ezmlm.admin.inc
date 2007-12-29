<?php
// $Id$
// $Name$
// for drupal 6.x



/**
 * Menu callback; presents the settings form for ezmlm
 */
function ezmlm_admin_settings() {
#  $form = array();
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
    if(is_array($lists) ) {
      foreach ($lists as $list_text => $list_email) {
        $form['ezmlm_delete'][$list_text] = array(
          '#type' => 'checkbox',
          '#title' => "$list_text ($list_email)" ,
        );
      }
    }
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
  $form['ezmlm_misc']['ezmlm_block_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Block title'),
    '#description' => t('Configure the title of the block subscription list'),
    '#default_value' => variable_get('ezmlm_block_title', t('Subscriptions')),
    '#size' => 20,
    '#maxlength' => 20,
    '#required' => FALSE,
  );

  return system_settings_form($form);
}

/**
 * Validate form
 */
function ezmlm_admin_settings_validate($form, &$form_state) {

  if (strlen($form_state['values']['ezmlm_add_address'])) {
    if ($error = user_validate_mail($form_state['values']['ezmlm_add_address'])) {
      form_set_error('ezmlm_add_address', $error);
    }
    if (! strlen($form_state['values']['ezmlm_add_name'])) {
      form_set_error('ezmlm_add_name', t('You must fill in the Name field'));
    }
  }
}

/**
 * Submit form
 */
function ezmlm_admin_settings_submit($form, &$form_state) {
  // various
  if ($form_state['values']['ezmlm_title'] != "") {
    variable_set('ezmlm_title', $form_state['values']['ezmlm_title']);
  }
  if ($form_state['values']['ezmlm_block_title'] != "") {
    variable_set('ezmlm_block_title', $form_state['values']['ezmlm_block_title']);
  }
  // look for deletes
  $lists = _ezmlm_get_lists();
  if (is_array($lists)) {
    // deletions
    $newlists = array();
    $delct = 0;
    foreach ($lists as $list_text => $list_email) {
      if (isset($form_state['values'][$list_text]) && $form_state['values'][$list_text] == 1) {
        $delct++;
        continue;     // skip copying this list to the new list
      }
      else {
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
  if (!is_array($lists) || count($lists) < 1 ) {
    $lists = array();
    variable_del('ezmlmmailinglists');
  }
  $list_text = trim($form_state['values']['ezmlm_add_name']);
  $list_email = trim($form_state['values']['ezmlm_add_address']);
  if (strlen($list_text) && strlen($list_email) ) {
    $lists[$list_text] = $list_email;
    variable_set('ezmlmmailinglists', $lists);
    drupal_set_message( t('The mailing lists have been saved.'));
  }
}
