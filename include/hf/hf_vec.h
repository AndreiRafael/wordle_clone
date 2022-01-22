#ifndef HF_VEC_H
#define HF_VEC_H

#define HF_VEC2_INIT_ZERO { .x=0.f, .y=0.f }
#define HF_VEC2_INIT_ONE { .x=1.f, .y=1.f }
#define HF_VEC2_INIT_RIGHT { .x=1.f, .y=0.f }
#define HF_VEC2_INIT_LEFT { .x=-1.f, .y=0.f }
#define HF_VEC2_INIT_UP { .x=0.f, .y=1.f }
#define HF_VEC2_INIT_DOWN { .x=0.f, .y=-1.f }

typedef struct hf_vec2 {
    float x;
    float y;
} hf_vec2;


hf_vec2 hf_vec2_add(hf_vec2 left, hf_vec2 right);
hf_vec2 hf_vec2_sub(hf_vec2 left, hf_vec2 right);
hf_vec2 hf_vec2_mul(hf_vec2 v, float scalar);
hf_vec2 hf_vec2_div(hf_vec2 v, float scalar);

#endif//HF_VEC_H