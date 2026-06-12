#include <cmocka.h>
#include <stdarg.h>
#include <stddef.h>

#include "plugin-api.h"
#include "plugin-host.h"

/* ============================================================
 * Helpers
 * ============================================================ */

static Variable make_i64(int64_t v) {
  Variable var = {0};
  var.type = PARAM_TYPE_INT64;
  var.value.i64_val = v;
  return var;
}

static Variable make_double(double v) {
  Variable var = {0};
  var.type = PARAM_TYPE_DOUBLE;
  var.value.d_val = v;
  return var;
}

/* ============================================================
 * Basic lifecycle tests
 * ============================================================ */

static void test_create_free(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();
  assert_non_null(resp);

  assert_int_equal(plugin_response_count(resp), 0);

  plugin_response_free(resp);
}

static void test_reset(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();

  Variable v = make_i64(42);
  plugin_response_push(resp, &v);

  assert_int_equal(plugin_response_count(resp), 1);

  plugin_response_reset(resp);

  assert_int_equal(plugin_response_count(resp), 0);

  plugin_response_free(resp);
}

/* ============================================================
 * Push + Get correctness
 * ============================================================ */

static void test_push_single(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();

  Variable v = make_i64(123);
  assert_int_equal(plugin_response_push(resp, &v), 0);

  assert_int_equal(plugin_response_count(resp), 1);

  const Variable *out = plugin_response_get(resp, 0);
  assert_non_null(out);

  assert_int_equal(out->type, PARAM_TYPE_INT64);
  assert_int_equal(out->value.i64_val, 123);

  plugin_response_free(resp);
}

static void test_push_multiple(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();

  for (int i = 0; i < 5; i++) {
    Variable v = make_i64(i);
    assert_int_equal(plugin_response_push(resp, &v), 0);
  }

  assert_int_equal(plugin_response_count(resp), 5);

  for (int i = 0; i < 5; i++) {
    const Variable *out = plugin_response_get(resp, i);
    assert_non_null(out);
    assert_int_equal(out->value.i64_val, i);
  }

  plugin_response_free(resp);
}

/* ============================================================
 * SBO → heap transition
 * ============================================================ */

static void test_inline_to_heap_transition(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();

  /* Inline cap is 1 → push 2 elements */
  Variable a = make_i64(1);
  Variable b = make_i64(2);

  plugin_response_push(resp, &a);
  plugin_response_push(resp, &b);

  assert_int_equal(plugin_response_count(resp), 2);

  const Variable *v0 = plugin_response_get(resp, 0);
  const Variable *v1 = plugin_response_get(resp, 1);

  assert_int_equal(v0->value.i64_val, 1);
  assert_int_equal(v1->value.i64_val, 2);

  plugin_response_free(resp);
}

/* ============================================================
 * Reserve behavior
 * ============================================================ */

static void test_reserve_prevents_growth(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();

  plugin_response_reserve(resp, 10);

  for (int i = 0; i < 10; i++) {
    Variable v = make_i64(i);
    assert_int_equal(plugin_response_push(resp, &v), 0);
  }

  assert_int_equal(plugin_response_count(resp), 10);

  for (int i = 0; i < 10; i++) {
    const Variable *out = plugin_response_get(resp, i);
    assert_int_equal(out->value.i64_val, i);
  }

  plugin_response_free(resp);
}

/* ============================================================
 * Bounds checking
 * ============================================================ */

static void test_get_out_of_bounds(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();

  Variable v = make_i64(5);
  plugin_response_push(resp, &v);

  assert_null(plugin_response_get(resp, 1));
  assert_null(plugin_response_get(resp, 100));

  plugin_response_free(resp);
}

/* ============================================================
 * Null safety
 * ============================================================ */

static void test_null_safety(void **state) {
  (void)state;

  assert_int_equal(plugin_response_count(NULL), 0);
  assert_null(plugin_response_get(NULL, 0));

  Variable v = make_i64(1);

  assert_int_not_equal(plugin_response_push(NULL, &v), 0);
  assert_int_not_equal(plugin_response_push(NULL, NULL), 0);
}

/* ============================================================
 * Data isolation (copy semantics)
 * ============================================================ */

static void test_copy_semantics(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();

  Variable v = make_i64(10);
  plugin_response_push(resp, &v);

  /* mutate original */
  v.value.i64_val = 999;

  const Variable *stored = plugin_response_get(resp, 0);

  /* must not change */
  assert_int_equal(stored->value.i64_val, 10);

  plugin_response_free(resp);
}

/* ============================================================
 * Mixed types
 * ============================================================ */

static void test_mixed_types(void **state) {
  (void)state;

  PluginResponse *resp = plugin_response_create();

  Variable a = make_i64(42);
  Variable b = make_double(3.14);

  plugin_response_push(resp, &a);
  plugin_response_push(resp, &b);

  const Variable *va = plugin_response_get(resp, 0);
  const Variable *vb = plugin_response_get(resp, 1);

  assert_int_equal(va->type, PARAM_TYPE_INT64);
  assert_int_equal(vb->type, PARAM_TYPE_DOUBLE);

  assert_int_equal(va->value.i64_val, 42);
  assert_true(vb->value.d_val > 3.13);

  plugin_response_free(resp);
}

/* ============================================================
 * Main entry
 * ============================================================ */

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_create_free),
      cmocka_unit_test(test_reset),
      cmocka_unit_test(test_push_single),
      cmocka_unit_test(test_push_multiple),
      cmocka_unit_test(test_inline_to_heap_transition),
      cmocka_unit_test(test_reserve_prevents_growth),
      cmocka_unit_test(test_get_out_of_bounds),
      cmocka_unit_test(test_null_safety),
      cmocka_unit_test(test_copy_semantics),
      cmocka_unit_test(test_mixed_types),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
