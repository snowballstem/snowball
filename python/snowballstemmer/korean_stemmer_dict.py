#!/usr/bin/env python3
"""
한국어 사전 기반 stemmer wrapper

기존 Snowball KoreanStemmer에 사전 lookup 레이어를 추가합니다.

사용법:
    from snowballstemmer.korean_stemmer_dict import KoreanStemmerDict
    stemmer = KoreanStemmerDict()
    result = stemmer.stem("학교에서")  # 사전에 있으면 원형 반환, 아니면 stemmer 처리
"""

import os
import json
import pickle
import time
from typing import Dict, Optional, Set

# Snowball 컴파일러에서 생성된 stemmer import
from .korean_stemmer import KoreanStemmer as _SnowballKoreanStemmer


# ============================================================================
# 격 조사 목록 (격 조사 제거용)
# ============================================================================
CASE_MARKERS = frozenset([
    # 주격/목적격 조사
    "에서", "을", "를", "의",
    # 병격/방향 조사
    "과", "와", "으로", "로", "에",
    # 부사격 조사
    "도", "만", "조차", "라도",
    # 복합 격 조사
    "이라도", "조차도",
    # 범위/비교 조사
    "까지", "부터", "처럼",
])

# ============================================================================
# 용언 접미사 규칙 (접미사 제거용)
# ============================================================================
# 키: 제거할 접미사, 값: 제거 후 남을 어간 (빈 문자열은 완전 제거)
VERBAL_SUFFIXES = frozenset([
    # 종결 어미
    "습니다", "어요", "네", "라", "자", "세요",
    # 과거 시제 어미 (+다) - ㄹ불규칙
    "랐다",
    # 과거 시제 어미 (+다)
    "았다", "었다", "였다",
    # 과거 시제 어미 (단어 끝이 아닌 경우)
    "았", "었", "였", "랐",
    # 연결 어미
    "고", "니", "니까", "어서", "아", "야",
    # 동사/형용사 종결 어미
    "다",
])


class KoreanStemmerDict:
    """
    사전 기반 lookup 레이어가 있는 한국어 stemmer.

    파이프라인:
        입력 단어 → [사전 lookup] → 사전에 있으면 원형 즉시 반환
                            → 아니면 기존 Snowball stemmer 처리

    사전 로드:
        1. kiwipiepy의 내장 사전 (default.dict)
        2. JSON 파일
        3. pickle 파일
        4. 내장 사전 (소규모 테스트용)
    """

    def __init__(
        self,
        dict_source: str = "kiwi",
        dict_path: Optional[str] = None,
        dict_format: str = "json",
        use_builtin_fallback: bool = True,
    ):
        """
        Args:
            dict_source: 사전 소스 ('kiwi', 'json', 'pickle', 'builtin')
            dict_path: 사전 파일 경로 (dict_source가 'json' 또는 'pickle'일 때)
            dict_format: 사전 파일 형식 ('json', 'pickle')
            use_builtin_fallback: 사전 로드 실패 시 내장 사전 사용
        """
        self._stemmer = _SnowballKoreanStemmer()
        self._lemma_map: Dict[str, str] = {}
        self._word_set: Set[str] = set()
        self._irregular_map: Dict[str, str] = {}  # 불규칙 용언 매핑
        self._dict_source = dict_source
        self._dict_path = dict_path
        self._load_time = 0.0

        # 불규칙 용언 사전 항상 로드
        self._load_irregular_dict()

        # 사전 로드
        self._load_dict(dict_source, dict_path, dict_format, use_builtin_fallback)

    def _load_dict(
        self,
        dict_source: str,
        dict_path: Optional[str],
        dict_format: str,
        use_builtin_fallback: bool,
    ):
        """사전을 로드합니다."""
        start_time = time.time()

        if dict_source == "kiwi":
            self._load_kiwi_dict()
        elif dict_source == "json":
            if dict_path is None:
                dict_path = os.path.join(
                    os.path.dirname(__file__),
                    "..",
                    "..",
                    "data",
                    "korean_dict.json"
                )
            self._load_json_dict(dict_path)
        elif dict_source == "pickle":
            if dict_path is None:
                dict_path = os.path.join(
                    os.path.dirname(__file__),
                    "..",
                    "..",
                    "data",
                    "korean_dict.pkl"
                )
            self._load_pickle_dict(dict_path)
        elif dict_source == "builtin":
            self._load_builtin_dict()
        else:
            raise ValueError(f"지원하지 않는 사전 소스: {dict_source}")

        self._load_time = time.time() - start_time

        if not self._lemma_map:
            if use_builtin_fallback:
                print("[경고] 사전 로드가 실패했습니다. 내장 사전을 사용합니다.")
                self._load_builtin_dict()
            else:
                print("[경고] 사전 로드가 실패했습니다. stemmer만 사용합니다.")

    def _load_kiwi_dict(self):
        """kiwipiepy의 내장 사전을 로드합니다."""
        try:
            dict_path = self._find_kiwi_dict_path()
            if dict_path is None:
                raise FileNotFoundError("kiwipiepy default.dict not found")
            self._parse_kiwi_dict(dict_path)
        except Exception as e:
            print(f"[경고] kiwipiepy 사전 로드 실패: {e}")
            self._lemma_map = {}
            self._word_set = set()

    def _find_kiwi_dict_path(self) -> Optional[str]:
        """kiwipiepy의 default.dict 파일 경로를 찾습니다."""
        try:
            import kiwipiepy
            kiwi_dir = os.path.dirname(kiwipiepy.__file__)
            model_dir = os.path.join(
                os.path.dirname(kiwi_dir),
                "kiwipiepy_model",
                "default.dict"
            )
            if os.path.exists(model_dir):
                return model_dir
        except ImportError:
            pass

        # pip install 경로 확인
        import site
        for site_pkg in site.getsitepackages():
            model_dir = os.path.join(
                site_pkg,
                "kiwipiepy_model",
                "default.dict"
            )
            if os.path.exists(model_dir):
                return model_dir

        return None

    def _parse_kiwi_dict(self, dict_path: str):
        """kiwipiepy의 default.dict 파일을 파싱합니다."""
        self._lemma_map = {}
        self._word_set = set()

        with open(dict_path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                if "\t" not in line:
                    continue

                parts = line.split("\t")
                word = parts[0]

                # 빈 단어 건너뛰기
                if not word:
                    continue

                # 한국어 문자만 포함하는지 확인
                if not any('\uAC00' <= c <= '\uD7A3' for c in word):
                    continue

                # 형태소 분석 결과인 경우
                if len(parts) >= 2 and " +" in parts[1]:
                    morphemes = parts[1].split(" + ")
                    for morph in morphemes:
                        morph = morph.strip()
                        if "/" in morph:
                            lemma = morph.split("/")[0]
                            if lemma and '\uAC00' <= lemma[0] <= '\uD7A3':
                                self._lemma_map[lemma] = lemma
                                self._word_set.add(lemma)
                    # 전체 단어 추가
                    self._word_set.add(word)
                    continue

                # 단순 형태소
                if len(parts) >= 2:
                    pos_tag = parts[1].strip()
                    # 명사, 용언, 부사, 조사, 접미사, 어미 등 추가
                    if pos_tag.startswith(("NN", "VV", "VA", "VX", "MAG",
                                           "EC", "EP", "EF", "JX", "JK",
                                           "SL", "SW", "XSN", "XSA", "XSV")):
                        self._lemma_map[word] = word
                        self._word_set.add(word)

    def _load_json_dict(self, path: str):
        """JSON 파일에서 사전을 로드합니다."""
        with open(path, "r", encoding="utf-8") as f:
            self._lemma_map = json.load(f)
        self._word_set = set(self._lemma_map.keys())

    def _load_pickle_dict(self, path: str):
        """pickle 파일에서 사전을 로드합니다."""
        with open(path, "rb") as f:
            self._lemma_map = pickle.load(f)
        self._word_set = set(self._lemma_map.keys())

    def _load_builtin_dict(self):
        """내장 사전을 로드합니다."""
        self._lemma_map = {
            # 명사
            "학교": "학교", "학생": "학생", "사람": "사람", "책": "책",
            "친구": "친구", "가족": "가족", "일": "일", "시간": "시간",
            "물건": "물건", "의사": "의사", "선생님": "선생님",
            # 동사
            "먹다": "먹다", "가다": "가다", "오다": "오다", "하다": "하다",
            "보다": "보다", "듣다": "듣다", "걷다": "걷다", "짓다": "짓다",
            "놀다": "놀다", "돕다": "돕다", "받다": "받다", "웃다": "웃다",
            "붓다": "붓다", "낫다": "낫다", "믿다": "믿다", "갚다": "갚다",
            # 형용사
            "크다": "크다", "작다": "작다", "좋다": "좋다", "나쁘다": "나쁘다",
            "빠르다": "빠르다", "느리다": "느리다", "길다": "길다", "짧다": "짧다",
            # 부사
            "매우": "매우", "아주": "아주", "너무": "너무", "조금": "조금",
            "많이": "많이", "적게": "적게", "빨리": "빨리", "천천히": "천천히",
            # 조사
            "에서": "에서", "에": "에", "을": "을", "를": "를",
            "의": "의", "와": "와", "과": "과", "도": "도", "만": "만",
            "부터": "부터", "까지": "까지", "처럼": "처럼", "조차": "조차",
            "라도": "라도", "으로": "으로", "로": "로",
            # 접미사
            "들": "들", "이": "이", "음": "음", "함": "함",
            # 어미
            "습니다": "습니다", "어요": "어요", "다": "다", "고": "고",
            "니": "니", "자": "자", "라": "라", "세요": "세요",
            "었": "었", "았": "았", "겠": "겠",
        }
        self._word_set = set(self._lemma_map.keys())

    def _load_irregular_dict(self):
        """불규칙 용언 매핑 사전을 로드합니다.
        
        data/irregular_verb_dict.json 파일을 로드하여
        활용형 -> 어근 매핑을 _irregular_map에 저장합니다.
        """
        try:
            # 프로젝트 루트의 data/ 디렉토리에서 로드
            base_dir = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
            dict_path = os.path.join(base_dir, "data", "irregular_verb_dict.json")
            
            if os.path.exists(dict_path):
                with open(dict_path, "r", encoding="utf-8") as f:
                    self._irregular_map = json.load(f)
            else:
                # 파일이 없으면 빈 사전으로 시작
                self._irregular_map = {}
                print(f"[경고] 불규칙 용언 사전 파일을 찾을 수 없습니다: {dict_path}")
        except Exception as e:
            print(f"[경고] 불규칙 용언 사전 로드 실패: {e}")
            self._irregular_map = {}

    def stem(self, word: str) -> str:
        """
        단일 단어의 stem을 반환합니다.

        파이프라인:
            1. 격 조사 제거: word가 조사로 끝나면 조사 제거 후 반환 (우선순위 높음)
            2. 불규칙 용언 사전 lookup: word가 불규칙 사전에 있으면 어근 즉시 반환
            3. 용언 접미사 제거: word가 용언 접미사로 끝나면 접미사 제거 후 반환
            4. 일반 사전 lookup: word가 일반 사전에 있으면 원형 즉시 반환
            5. Snowball stemmer: 위 단계 모두 실패 시 기존 stemmer 처리

        Args:
            word: stem할 한국어 단어

        Returns:
            stem (원형 또는 규칙 기반 stem)
        """
        # 1. 격 조사 제거 (긴 조사부터 검사) — 불규칙 사전 충돌 방지
        for marker in sorted(CASE_MARKERS, key=len, reverse=True):
            if word.endswith(marker) and len(word) > len(marker):
                return word[:-len(marker)]

        # 2. 불규칙 용언 사전 먼저 확인
        if word in self._irregular_map:
            return self._irregular_map[word]

        # 3. 용언 접미사 제거 (긴 접미사부터 검사)
        # 다/았다/었다/였다 먼저 제거 → 어간 추출
        for suffix in sorted(VERBAL_SUFFIXES, key=len, reverse=True):
            if word.endswith(suffix) and len(word) > len(suffix):
                return word[:-len(suffix)]

        # 4. 일반 사전 lookup
        if word in self._word_set:
            return self._lemma_map.get(word, word)

        # 5. Snowball stemmer 처리
        self._stemmer.set_current(word)
        self._stemmer._stem()
        return self._stemmer.get_current()

    def stemWords(self, words: list) -> list:
        """
        여러 단어의 stem을 반환합니다.

        Args:
            words: stem할 한국어 단어 목록

        Returns:
            stem 목록
        """
        return [self.stem(word) for word in words]

    @property
    def dict_size(self) -> int:
        """사전의 단어 수."""
        return len(self._lemma_map)

    @property
    def load_time(self) -> float:
        """사전 로딩 시간 (초)."""
        return self._load_time

    @property
    def dict_source(self) -> str:
        """사전 소스."""
        return self._dict_source

    def __repr__(self):
        return (
            f"KoreanStemmerDict(dict_source='{self._dict_source}', "
            f"dict_size={self.dict_size}, load_time={self._load_time:.3f}s)"
        )


# 편의 함수
def stem(word: str, dict_source: str = "kiwi") -> str:
    """
    편의 함수: 단어의 stem을 반환합니다.

    Args:
        word: stem할 한국어 단어
        dict_source: 사전 소스 ('kiwi', 'json', 'pickle', 'builtin')

    Returns:
        stem (원형 또는 규칙 기반 stem)
    """
    stemmer = KoreanStemmerDict(dict_source=dict_source)
    return stemmer.stem(word)


def stemWords(words: list, dict_source: str = "kiwi") -> list:
    """
    편의 함수: 여러 단어의 stem을 반환합니다.

    Args:
        words: stem할 한국어 단어 목록
        dict_source: 사전 소스 ('kiwi', 'json', 'pickle', 'builtin')

    Returns:
        stem 목록
    """
    stemmer = KoreanStemmerDict(dict_source=dict_source)
    return stemmer.stemWords(words)
