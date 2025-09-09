#ifndef NLP_PROCESSOR_H
#define NLP_PROCESSOR_H

#include "cJSON.h"
#include "ai_assistant.h"

// NLP处理配置
#define NLP_MAX_TEXT_LEN 512
#define NLP_MAX_INTENT_LEN 64
#define NLP_MAX_ENTITY_LEN 128
#define NLP_MAX_ENTITIES 10

// NLP处理状态
typedef enum {
    NLP_PROCESSOR_STATE_IDLE = 0,
    NLP_PROCESSOR_STATE_PROCESSING,
    NLP_PROCESSOR_STATE_ERROR
} nlp_processor_state_t;

// 实体类型
typedef enum {
    NLP_ENTITY_TYPE_UNKNOWN = 0,
    NLP_ENTITY_TYPE_PERSON,
    NLP_ENTITY_TYPE_LOCATION,
    NLP_ENTITY_TYPE_TIME,
    NLP_ENTITY_TYPE_NUMBER,
    NLP_ENTITY_TYPE_ORGANIZATION,
    NLP_ENTITY_TYPE_MONEY,
    NLP_ENTITY_TYPE_PERCENT
} nlp_entity_type_t;

// 实体结构
typedef struct {
    char text[NLP_MAX_ENTITY_LEN];
    nlp_entity_type_t type;
    float confidence;
    uint32_t start_pos;
    uint32_t end_pos;
} nlp_entity_t;

// 意图识别结果
typedef struct {
    char intent[NLP_MAX_INTENT_LEN];
    float confidence;
    nlp_entity_t entities[NLP_MAX_ENTITIES];
    uint8_t entity_count;
    char original_text[NLP_MAX_TEXT_LEN];
} nlp_intent_result_t;

// NLP处理器配置结构
typedef struct {
    uint8_t language;
    bool enable_entity_recognition;
    bool enable_sentiment_analysis;
    bool enable_keyword_extraction;
    float confidence_threshold;
    char model_path[128];
} nlp_processor_config_t;

// 函数声明
esp_err_t nlp_processor_init(void);
esp_err_t nlp_processor_set_config(const nlp_processor_config_t *config);
esp_err_t nlp_processor_get_config(nlp_processor_config_t *config);
voice_command_type_t nlp_processor_classify_command(const char *text);
esp_err_t nlp_processor_extract_intent(const char *text, char *intent, size_t intent_len);
esp_err_t nlp_processor_extract_entities(const char *text, nlp_entity_t *entities, uint8_t *entity_count);
esp_err_t nlp_processor_analyze_sentiment(const char *text, float *sentiment_score);
esp_err_t nlp_processor_extract_keywords(const char *text, char keywords[][NLP_MAX_ENTITY_LEN], uint8_t *keyword_count);
nlp_processor_state_t nlp_processor_get_state(void);

// 内部函数
static esp_err_t nlp_processor_load_model(const char *model_path);
static esp_err_t nlp_processor_preprocess_text(const char *input, char *output, size_t output_len);
static esp_err_t nlp_processor_tokenize(const char *text, char tokens[][NLP_MAX_ENTITY_LEN], uint8_t *token_count);
static esp_err_t nlp_processor_remove_stopwords(const char *text, char *output, size_t output_len);
static esp_err_t nlp_processor_stem_words(const char *text, char *output, size_t output_len);
static esp_err_t nlp_processor_calculate_similarity(const char *text1, const char *text2, float *similarity);

#endif // NLP_PROCESSOR_H
