<?php

function taxonomy_walker_admin_page_view() {
  return drupal_get_form('taxonomy_walker_admin_form');
}

function taxonomy_walker_admin_form($form, &$form_state) {
  $vocabulary_options = taxonomy_walker_vocabulary_options();

  $vocabulary = isset($form_state['values']['taxonomy_term_vocabulary']) ? $form_state['values']['taxonomy_term_vocabulary'] : taxonomy_walker_default_vocabulary();

  $form['taxonomy_term_vocabulary'] = array(
    '#title' => t('Default starting vocabulary'),
    '#type' => 'select',
    '#options' => $vocabulary_options,
    '#default_value' => $vocabulary,

    // Bind an ajax callback to the change event (which is the default for the
    // select form type) of the first dropdown. It will replace the second
    // dropdown when rebuilt
    '#ajax' => array(
        // When 'event' occurs, Drupal will perform an ajax request in the
        // background. Usually the default value is sufficient (eg. change for
        // select elements), but valid values include any jQuery event,
        // most notably 'mousedown', 'blur', and 'submit'.
        'event' => 'change',
        'callback' => 'taxonomy_walker_admin_form_ajax_callback',
        'wrapper' => 'taxonomy-term-id-replace',
    ),
  );
  $form['taxonomy_term_id'] = array(
    '#prefix' => '<div id="taxonomy-term-id-replace">',
    '#suffix' => '</div>',
    // The entire enclosing div created here gets replaced when taxonomy_term_vocabulary
    // is changed.
    // when the form is rebuilt during ajax processing, the $vocabulary variable
    // will now have the new value and so the options will change    
    '#title' => t('Default starting taxonomy term ID'),
    '#type' => 'select',
    '#options' => taxonomy_walker_term_id_options($vocabulary),
    '#default_value' => isset($form_state['values']['taxonomy_term_id']) ? $form_state['values']['taxonomy_term_id'] : taxonomy_walker_default_term(),
  );
  $form['block_count'] = array(
    '#type' => 'textfield',
    '#title' => 'Number of blocks to generate',
    '#default_value' => taxonomy_walker_block_count(),
  );  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;  
}

/**
 * Selects just the second dropdown to be returned for re-rendering
 *
 * Since the controlling logic for populating the form is in the form builder
 * function, all we do here is select the element and return it to be updated.
 *
 * @return renderable array (the second dropdown)
 */
function taxonomy_walker_admin_form_ajax_callback($form, $form_state) {
    return $form['taxonomy_term_id'];
}

/* Options List Helper Functions */
function taxonomy_walker_term_id_options($vid = 11) {
  $terms = taxonomy_get_tree($vid, 0, 3);
  $term_options = array();
  foreach($terms as $key => $term) {
    $tid = $term->tid;
    $name = $term->name;
    $term_options[$tid] = $name;
  }
  return $term_options;
}
function taxonomy_walker_vocabulary_options() {
  $vocabularies = taxonomy_vocabulary_get_names();
  $vocabulary_options = array();
  foreach($vocabularies as $key => $vocabulary) {
    $vid = $vocabulary->vid;
    $name = $vocabulary->name;
    $vocabulary_options[$vid] = $name;
  }
  return $vocabulary_options;
}


function taxonomy_walker_admin_form_submit($form, $form_state) {
  taxonomy_walker_default_vocabulary($form_state['values']['taxonomy_term_vocabulary']);
  taxonomy_walker_default_term($form_state['values']['taxonomy_term_id']);
  taxonomy_walker_block_count($form_state['values']['block_count']);
}

