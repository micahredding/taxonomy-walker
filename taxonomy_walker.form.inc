<?php

function taxonomy_walker_form($form, &$form_state, $options = array()) {

  if(isset($form_state['values']['current_term_id'])) {
    $current_term_id = $form_state['values']['current_term_id'];
    $term_ids = $form_state['values']['term_ids'];
  } else {
    $current_term_id = $options['term_id'];
    $term_ids = array($current_term_id);
  } 

  if(isset($form_state['triggering_element'])) {
    switch($form_state['triggering_element']['#type']) {
      case 'submit':
        switch($form_state['triggering_element']['#value']) {
          case 'Next':
            $parent_term_id = $current_term_id;
            $current_term_id = $form_state['values']['questions'][$parent_term_id]['select'];
            $term_ids = _taxonomy_walker_update_term_ids($term_ids, $current_term_id, $parent_term_id);
            break;
          case 'Back':
            if(count($term_ids) > 1) {
              array_pop($term_ids);
              $current_term_id = $term_ids[count($term_ids) - 1];            
            }
            break;
          case 'Start Over':
            $current_term_id = $term_ids[0];
            $term_ids = array($current_term_id);
            break;
        }
        break;
      case 'image_button':
      case 'button':
      case 'select':
      case 'radio':
        $array = _taxonomy_walker_triggering_element_parse($form_state);
        $current_term_id = $array['current_term_id'];
        $parent_term_id = $array['parent_term_id'];
        $term_ids = _taxonomy_walker_update_term_ids($term_ids, $current_term_id, $parent_term_id);        
        break;
    }
  }
  $options = _taxonomy_walker_form_defaults($options);
  return _taxonomy_walker_form($term_ids, $current_term_id, $options);
}

function _taxonomy_walker_form($term_ids, $current_term_id, $options) {
  $options['ajax_wrapper'] = $options['form_id'];

  $form = array();
  $form['#prefix'] = '<div id="' . $options['form_id'] . '" class="form-step-' . (count($term_ids) - 1) . '">';
  $form['#suffix'] = '</div>';
  $form['#attributes'] = $options['#attributes'];

  $form['questions'] = _taxonomy_walker_form_questions($term_ids, $current_term_id, $options);
  $form['questions']['#tree'] = true;
  $form['questions']['#prefix'] = '<div id="questions" class="form-step-' . (count($term_ids) - 1) . '">';
  $form['questions']['#suffix'] = '</div>';

  $form['current_term_id'] = array(
    '#type' => 'hidden',
    '#value' => $current_term_id,
  );
  $form['term_ids'] = array(
    '#type' => 'hidden',
    '#value' => $term_ids,
  );

  return $form;
}

function _taxonomy_walker_form_questions($term_ids, $current_term_id, $options) {
  $questions = array();
  $starting_point = _taxonomy_walker_term_starting_point($term_ids, $options);

  for($i = $starting_point; $i < count($term_ids); $i++) {
    $buttons = $options['buttons'];
    $type = _taxonomy_walker_array_item($options['types'], $i);
    $question = _taxonomy_walker_form_question($term_ids[$i], $type, $options);

    /* If we have answers left to offer, it spits out the question and answers.
       Otherwise, it gives us the results */
    if($question) {
      if($question['buttons']) {
        $buttons = array_merge($buttons, $question['buttons']);
        unset($question['buttons']);
      }
      $questions[$term_ids[$i]] = $question;
    } else {
      taxonomy_walker_set_content($current_term_id);
      $questions[$term_ids[$i]]['content'] = taxonomy_walker_content($current_term_id);
    }
    $questions[$term_ids[$i]]['buttons'] = _taxonomy_walker_form_buttons($buttons, $options);
  }
  return $questions;  
}


function _taxonomy_walker_term_starting_point($term_ids, $options) {
  $starting_point = 0;
  if($options['terms_at_a_time'] > 0) {
    $starting_point = count($term_ids) - $options['terms_at_a_time'];
  }  
  return $starting_point;
}

function _taxonomy_walker_form_question($tid, $type, $options) {
  $question = taxonomy_walker_question_load($tid);
  $answers = taxonomy_walker_answers_load($tid);

  if(count($answers) == 0) {
    return false;
  }

  $answer = array();
  $answer['#prefix'] = '<div class="term-question term-question-' . $tid . ' term-question-type-' . $type . '">';
  $answer['#suffix'] = '</div>';

  switch($type) {
    case 'radios':
      $answer['select'] = array(
        '#title' => $question,
        '#type' => 'radios',
        '#options' => $answers,
      );
      $answer['select']['#ajax'] = array(
        'event' => 'click',
        'callback' => 'taxonomy_walker_form_ajax_callback',
        'wrapper' => $options['ajax_wrapper'],
      );
      break;    
    case 'button':
      $answer['select'] = array(
        '#title' => $question,
        '#type' => 'fieldset',
      );
      foreach ($answers as $tid => $term) {
        $answer['select'][$tid] = array(
          '#type' => 'button',
          '#value' => $term,
        );
        $answer['select'][$tid]['#ajax'] = array(
          'event' => 'click',
          'callback' => 'taxonomy_walker_form_ajax_callback',
          'wrapper' => $options['ajax_wrapper'],
        );
        if($tid == $options['current_term_id'] || in_array($tid, $options['term_ids'])) {
          $answer['select'][$tid]['#attributes'] = array(
            'class' => array('active'),
          );          
        }
      }
      break; 
    case 'image_button':
      $answer['select'] = array(
        '#title' => $question,
        '#type' => 'fieldset',
      );
      foreach ($answers as $tid => $term) {
        $answer['select'][$tid] = array(
          '#type' => 'container',
        );
        $answer['select'][$tid]['image'] = array(
          '#type' => 'image_button',
          '#name' => $term,
          '#return_value' => $tid,
          '#src' => file_create_url(taxonomy_walker_answer_as_image($tid)),
        );
        $answer['select'][$tid]['text'] = array(
          '#type' => 'button',
          '#value' => $term,
        );
        $answer['select'][$tid]['image']['#ajax'] = $answer['select'][$tid]['text']['#ajax'] = array(
          'event' => 'click',
          'callback' => 'taxonomy_walker_form_ajax_callback',
          'wrapper' => $options['ajax_wrapper'],
        );
        if($tid == $options['current_term_id'] || in_array($tid, $options['term_ids'])) {
          $answer['select'][$tid]['#attributes'] = array(
            'class' => array('active'),
          );          
        }
      }
      break;        
    case 'multiple':
    case 'checkboxes':
      $answer['select'] = array(
        '#title' => $question,
        '#type' => 'checkboxes',
        '#options' => $answers,
      );
      $answer['buttons'] = array('results');
      break;        
    case 'select':
    default:
      $answer['select'] = array(
        '#title' => $question,
        '#type' => 'select',
        '#options' => $answers,
        '#required' => TRUE,
        '#attributes' => array(
          'class' => array('happy'),
        ),
      );
      $answer['select']['#ajax'] = array(
        'event' => 'change',
        'callback' => 'taxonomy_walker_form_ajax_callback',
        'wrapper' => $options['ajax_wrapper'],
      );
      break;

  }

  return $answer;
}


function _taxonomy_walker_form_buttons($buttons, $options) {
  $source = array(
    'back' => array(
      '#type' => 'submit',
      '#value' => 'Back',
      '#submit' => array('taxonomy_walker_form_back'),
      '#ajax' => array(
        'event' => 'click',
        'callback' => 'taxonomy_walker_form_ajax_callback',
        'wrapper' => $options['ajax_wrapper'],
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => 'Next',
      '#submit' => array('taxonomy_walker_form_submit'),
      '#ajax' => array(
        'event' => 'click',
        'callback' => 'taxonomy_walker_form_ajax_callback',
        'wrapper' => $options['ajax_wrapper'],
      ),
    ),    
    'results' => array(
      '#type' => 'submit',
      '#value' => 'Continue to results...',
      '#submit' => array('taxonomy_walker_form_results'),
      '#attributes' => array(
        'class' => array(
          'submit-results',
          'buttons',
        ),
      ),
    ),
    'restart' => array(
      '#type' => 'submit',
      '#value' => 'Start Over',
      '#submit' => array('taxonomy_walker_form_restart'),
      '#attributes' => array(
        'class' => array(
          'submit-restart',
        ),
      ),
      '#ajax' => array(
        'event' => 'click',
        'callback' => 'taxonomy_walker_form_ajax_callback',
        'wrapper' => $options['ajax_wrapper'],
      ),
    ),
  );

  $buttons = _taxonomy_walker_include_options($buttons, $source);
  $buttons['#prefix'] = '<div class="buttons">';
  $buttons['#suffix'] = '</div>';

  return $buttons;
}

function _taxonomy_walker_form_defaults($options = array()) {
  $default_options = array(
    'type' => 'select',
    'types' => array(),
    'buttons' => array('back','submit','restart'),
    'form_id' => 'taxonomy-walker-form',
    'current_term_id' => 0,
    'term_ids' => array(0),
    'terms_at_a_time' => 0,
    '#attributes' => array(),
  );
  $options = array_merge($default_options, $options);
  if(!$options['buttons']) $options['buttons'] = array();
  return $options;
}


/* Form Handlers */
function taxonomy_walker_form_submit($form, &$form_state) {  
  $form_state['rebuild'] = true;
}
function taxonomy_walker_form_back($form, &$form_state) { 
  $form_state['rebuild'] = true;
}
function taxonomy_walker_form_restart($form, &$form_state) { 
  $form_state['rebuild'] = true;
}
function taxonomy_walker_form_results($form, &$form_state) { 
  // drupal_goto('found');
  // $form_state['rebuild'] = true;
  $current_term_id = $form_state['values']['current_term_id'];
  $parent_term_id = $current_term_id;
  $current_term_id = $form_state['values']['questions'][$parent_term_id]['select'];
  taxonomy_walker_set_content($current_term_id);

  $form_state['redirect'] = 'found';
}
function taxonomy_walker_form_ajax_callback($form, $form_state) {
  return $form;
}
