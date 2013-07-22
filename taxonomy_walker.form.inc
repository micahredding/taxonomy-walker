<?php

function taxonomy_walker_form($form, &$form_state, $options = array()) {

  if(isset($form_state['values']['current_term_id'])) {
    $current_term_id = $form_state['values']['current_term_id'];
    $term_ids = $form_state['values']['term_ids'];
  } else {
    $current_term_id = isset($options['term_id']) ? $options['term_id'] : taxonomy_walker_default_term();
    $term_ids = array($current_term_id);
  } 

  if(isset($form_state['triggering_element'])) {
    switch($form_state['triggering_element']['#type']) {
      case 'submit':
        switch($form_state['triggering_element']['#value']) {
          case 'Next':
            $parent_term_id = $current_term_id;
            $current_term_id = $form_state['values']['answer'][$parent_term_id]['select'];
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
  $form['answer'] = array(
    '#tree' => true,
  );
  $form['current_term_id'] = array(
    '#type' => 'hidden',
    '#value' => $current_term_id,
  );
  $form['term_ids'] = array(
    '#type' => 'hidden',
    '#value' => $term_ids,
  );

  if($options['terms_at_a_time'] > 0) {
    $starting_point = count($term_ids) - $options['terms_at_a_time'];
  } else {
    $starting_point = 0;
  }
  for($i = $starting_point; $i < count($term_ids); $i++) {
    $type = _taxonomy_walker_array_item($options['types'], $i);
    $term_id = $term_ids[$i];
    $answer = _taxonomy_walker_form_answer($term_id, $type, $options);
    if(count($answer) > 0) {
      $form['answer'][$term_id] = $answer;
      $form['answer'][$term_id]['#prefix'] = '<div class="term-question term-question-' . $term_id . ' term-question-type-' . $type . '">';
      $form['answer'][$term_id]['#suffix'] = '</div>';
    }
  }

  if($options['include_content']) {
    $form['content'] = array(
      '#prefix' => '<div class="term-content">',
      '#suffix' => '</div>',
      'content' => taxonomy_walker_term_content($current_term_id),
    );    
  }

  if($options) {
    $buttons_source = _taxonomy_walker_form_buttons($options['ajax_wrapper']);
    $buttons_select = _taxonomy_walker_include_options($options['include_buttons'], $buttons_source);
    $form = array_merge($form, $buttons_select);
  }

  return $form;
}

function _taxonomy_walker_array_item($array, $i) {
  $count = count($array);
  $item = ($count > $i) ? $array[$i] : $array[$count - 1];
  return $item;
}

function _taxonomy_walker_form_answer($tid, $type, $options) {
  $question = taxonomy_walker_question_load($tid);
  $answers = taxonomy_walker_answers_load($tid);

  if(count($answers) == 0) {
    return $answers;
  }
  $answer = array();

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


function _taxonomy_walker_form_buttons($ajax_wrapper) {
  $buttons = array(
    'back' => array(
      '#type' => 'submit',
      '#value' => 'Back',
      '#submit' => array('taxonomy_walker_form_back'),
      '#ajax' => array(
        'event' => 'click',
        'callback' => 'taxonomy_walker_form_ajax_callback',
        'wrapper' => $ajax_wrapper,
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => 'Next',
      '#submit' => array('taxonomy_walker_form_submit'),
      '#ajax' => array(
        'event' => 'click',
        'callback' => 'taxonomy_walker_form_ajax_callback',
        'wrapper' => $ajax_wrapper,
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
        'wrapper' => $ajax_wrapper,
      ),
    ),    
  );

  return $buttons;
}

function _taxonomy_walker_form_defaults($options = array()) {
  $default_options = array(
    'type' => 'select',
    'types' => array(),
    'include_content' => true,
    'include_buttons' => array('back','submit','restart'),
    'form_id' => 'taxonomy-walker-form',
    'current_term_id' => 0,
    'term_ids' => array(0),
    'terms_at_a_time' => 0,
    '#attributes' => array(),
  );
  return array_merge($default_options, $options);
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
function taxonomy_walker_form_ajax_callback($form, $form_state) {
  return $form;
}
