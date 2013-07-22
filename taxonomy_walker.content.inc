<?php

function taxonomy_walker_load_content() {
  $terms = $_COOKIE['Drupal_visitor_taxonomy_walker_tids'];
  $term_ids = explode(',', $terms);
  dpm($terms);
  return taxonomy_walker_content($term_ids);
}

function taxonomy_walker_set_content($term_ids) {
  user_cookie_save(array('taxonomy_walker_tids' => implode(',', $term_ids)));
}

function taxonomy_walker_content($term_ids) {
  if(count($term_ids) == 0) return array();

  $form = array();
  $form['#prefix'] = '<div class="term-content">';
  $form['#suffix'] = '</div>';

  foreach($term_ids as $index => $tid) {
    $form[$tid] = array();
    $form[$tid]['#prefix'] = '<div class="term-content-' . $tid . '">';
    $form[$tid]['#suffix'] = '</div>';    
    $form[$tid]['content'] = taxonomy_walker_term_content($tid);
  }

  return $form;
}

function taxonomy_walker_content_form(&$form, &$form_state, $options) {
  return taxonomy_walker_load_content();
}