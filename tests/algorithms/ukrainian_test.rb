# Words test set
# https://raw.githubusercontent.com/snowballstem/snowball-data/master/russian/voc.txt

# Граматика української мови
# https://uk.wikipedia.org/wiki/%D0%93%D1%80%D0%B0%D0%BC%D0%B0%D1%82%D0%B8%D0%BA%D0%B0_%D1%83%D0%BA%D1%80%D0%B0%D1%97%D0%BD%D1%81%D1%8C%D0%BA%D0%BE%D1%97_%D0%BC%D0%BE%D0%B2%D0%B8


# 1
# runtime/utilities.c
# comment out 'debug' function:
  #if 1
        # fprintf(stderr, "faulty slice operation:\n");
        # debug(z, -1, 0);

  #if 1
  #extern void debug(struct SN_env * z, int number, int line_count) {

# insert 'debug' where debugging is needed:
# src_c/stem_UTF_8_ukrainian.c
# static int r_perfective_gerund(struct SN_env * z) {
  # printf("R1: ");
  # printf("R2: ");
  # debug(z, -1, 0);

# 2
# bin/utf_to_sbl ./explanations/ukrainian.sbl.utf > algorithms/ukrainian.sbl && make


# 4
# ruby tests/algorithms/ukrainian_test.rb
# ./snowball algorithms/ukrainian_test.sbl -syntax
# ./snowball algorithms/english.sbl -syntax


perfective_gerund = {
  'вши'    => %w[написа|вши    зроби|вши],
  'вшись'  => %w[оберіга|вшись спакува|вшись],
  'вшися'  => %w[оберіга|вшися сподіва|вшися],
    # (а и )
  'учи'    => %w[пиш|учи       рев|учи],
  'ючи'    => %w[ся|ючи        підпису|ючи],
  'ючись'  => %w[змага|ючись],
  'ючися'  => %w[навча|ючися],
  'ачи'    => %w[бач|ачи],
  'ачись'  => %w[бач|ачись],
  'ачися'  => %w[бач|ачися],
  'лячи'   => %w[роб|лячи      люб|лячи],
  'лячись' => %w[бав|лячись],
  'лячися' => %w[бав|лячися],
  'ячи'    => %w[сид|ячи       вовтуз|ячи],
  'ячись'  => %w[вовтуз|ячись]
}

reflexive = {
  'ється' => %w[смі|ється], # незнайшов дієслів
  'еться' => %w[спиш|еться], # незнайшов дієслів

   # *ся
  'ся'    => %w[осік|ся],

  # rules from verb:
  'ти'  => %w[би|ти.ся вовтузи|ти.ся],
  # 'ть'  => %w[роб|ить.ся], # вже є правило "ить"
  'іть' => %w[подруж|іть.ся],
  'те'  => %w[бав|те.ся],
  'й'   => %w[вдума|й.ся],
  'йте' => %w[вдума|йте.ся],
  'ать' => %w[бач|ать.ся],

  'в ла ло ли'             => %w[вчи|в.ся   вчи|ла.ся  вчи|ло.ся    вчи|ли.ся],
  'ю єш ємо єте ють'       => %w[смі|ю.ся   смі|єш.ся  смі|ємо.ся   смі|єте.ся   смі|ють.ся],
  'у еш  емо ете уть'      => %w[спиш|у.ся  спиш|еш.ся спиш|емо.ся  спиш|ете.ся  спиш|уть.ся],
  'лю иш ить имо ите лять' => %w[див|лю.ся  див|иш.ся  див|ить.ся   див|имо.ся   див|ите.ся   див|лять.ся],
  'їш їть їмо їте ять'     => %w[бо|їш.ся   бо|їть.ся  бо|їмо.ся    бо|їте.ся    бо|ять.ся],


   # *сь
  'сь'    => %w[недонавч|и.сь змага|ю.сь], # вчи|сь

  # rules from verb:
  '2 ти'         => %w[боро|ти.сь],
  '2 в ла ло ли' => %w[вчи|в.сь   вчи|ла.сь  вчи|ло.сь    вчи|ли.сь],
}

# do we need to add prefix rule 'н*' for adjectives?
# because removes single vowels for verbs like 'вчи|ла.сь' = > 'вчи|л'
adjective = {
  'ий ого ому им ім'            => %w[зел|ен.ий    зел|ен.ого   зел|ен.ому   зел|ен.им    зел|ен.ім],
  'іший ішого ішому ішим ішім'  => %w[зел|ен.іший  зел|ен.ішого зел|ен.ішому зел|ен.ішим  зел|ен.ішім],
  'іше'                         => %w[зел|ен.іше],
  'ої ій ою'                    => %w[зел|ен.ої    зел|ен.ій    зел|ен.ою],
  'іша ішої ішій ішу ішою'      => %w[зел|ен.іша   зел|ен.ішої  зел|ен.ішій  зел|ен.ішу   зел|ен.ішою],
  'их ими'                      => %w[зел|ен.их    зел|ен.ими],
  'іші іших ішими'              => %w[зел|ен.іші   зел|ен.іших  зел|ен.ішими],
  'ього ьому'                   => %w[верхн|ього  верхн|ьому],
  'ьої ьою'                     => %w[верхн|ьої   верхн|ьою],
  'іх іми'                      => %w[верхн|іх    верхн|іми],
  'ова ове'                     => %w[ацетон|ова  ацетон|ове],
  'їй иїй'                      => %w[довгові|їй  голош|иїй],
  'йому ийому'                  => %w[незна|йому  товстош|ийому],
  'єє еє'                       => %w[трет|єє     турецьк|еє],
  'ена ені ене ену' => %w[втрач|ені   втрач|ена   втрач|ене    втрач|ену],
  'яча яче ячу ячі' => %w[сид|яча     сид|яче     сид|ячу      сид|ячі],
  'ача аче ачу ачі' => %w[дриж|ача    дриж|аче    дриж|ачу     дриж|ачі],
  'юча юче ючу ючі' => %w[далені|юча  далені|юче  далені|ючу   далені|ючі],
  'уча уче учу учі' => %w[пиш|уча     пиш|уче     пиш|учу      пиш|учі],
}

adjectival = {
  'яч'  => %w[сид|яч.ий    сид|яч.ого     сид|яч.ому     сид|яч.им      сид|яч.ім
              сид|яч.ої    сид|яч.ій      сид|яч.ою      сид|яч.их      сид|яч.ими],

  'ач'  => %w[дриж|ач.ий   дриж|ач.ого    дриж|ач.ому    дриж|ач.им     дриж|ач.ім
              дриж|ач.ої   дриж|ач.ій     дриж|ач.ою     дриж|ач.их     дриж|ач.ими],

  'юч'  => %w[далені|юч.ий далені|юч.ого  далені|юч.ому  далені|юч.им   далені|юч.ім
              далені|юч.ої далені|юч.ій   далені|юч.ою   далені|юч.их   далені|юч.ими],

  'уч'  => %w[пиш|уч.ий    далені|уч.ого  пиш|уч.ому     пиш|уч.им      пиш|уч.ім
              пиш|уч.ої    пиш|уч.ій      пиш|уч.ою      пиш|уч.их      пиш|уч.ими],

  'ен'  => %w[втрач|ен.ий  втрач|ен.ого   втрач|ен.ому   втрач|ен.им    втрач|ен.ім
              втрач|ен.ої  втрач|ен.ій    втрач|ен.ою    втрач|ен.их    втрач|ен.ими
              клен| ],
}

# https://en.wikipedia.org/wiki/Reflexive_pronoun
# https://en.wikipedia.org/wiki/Reflexive_verb
# https://uk.wikipedia.org/wiki/%D0%A0%D0%B5%D1%84%D0%BB%D0%B5%D0%BA%D1%81%D0%B8%D0%B2%D0%BD%D0%B5_%D0%B4%D1%96%D1%94%D1%81%D0%BB%D0%BE%D0%B2%D0%BE
# reflexive_full = {
#   # 'ся'    => %w[осік|ся],
#   # 'сь'    => %w[недонавч|и.сь], # вчи|сь
#   'тися'  => %w[би|тися милува|тися],
#   'тись'  => %w[боро|тись],
#   'ться'  => %w[милується|ться], # => роб|иться !? - бере завжди довший, треба інше слово підібрати або видалити якесь з правил
#   'іться' => %w[подруж|іться],
#   'теся'  => %w[бав|теся],
#   'йся'   => %w[вдума|йся],
#   'йтеся' => %w[вдума|йтеся],
#   'аться'                             => %w[бач|аться],
#   'вся лася лося лися'                => %w[вчи|вся   вчи|лася  вчи|лося    вчи|лися],
#   'всь лась лось лись'                => %w[вчи|всь   вчи|лась  вчи|лось    вчи|лись],
#   'юся єшся ється ємося єтеся ються'  => %w[смі|юся   смі|єшся  смі|ється   смі|ємося   смі|єтеся   смі|ються],
#   'уся ешся еться емося етеся уться'  => %w[спиш|уся  спиш|ешся спиш|еться  спиш|емося  спиш|етеся  спиш|уться],
#   'люся ишся иться имося итеся ляться'=> %w[див|люся  див|ишся  див|иться   див|имося   див|итеся   див|ляться],
#   'їшся їться їмося їтеся яться'      => %w[бо|їшся   бо|їться  бо|їмося    бо|їтеся    бо|яться],
# }

# reflexive_short = {
#   'сь'    => %w[вчи|сь
#                 недонавч|и.сь
#                 боро|ти.сь
#                 вчи|в.сь      вчи|ло.сь    вчи|ли.сь],
#                 # вчи|ла.сь - removes 'а' in adjective ?

#   'ся'    => %w[осік|ся
#                 би|ти.ся
#                 роб|иться
#                 подруж|іться
#                 бавт|е.ся
#                 вдума|йся
#                 вдумайт|еся
#                 вчи|в.ся     вчи|ла.ся вчи|ло.ся   вчи|ли.ся
#                 смі|юся      смі|єшся  смі|ється   смі|ємося   смі|єтеся   смі|ються
#                 спиш|уся     спиш|ешся спиш|еться  спиш|емося  спиш|етеся  спиш|уться
#                 див|люся     див|ишся  див|иться   див|имося   див|итеся   див|ляться
#                 бо|їшся      бо|їться  бо|їмося    бо|їтеся    бо|яться
#                 бач|аться],

#                 # бавт|е.ся # adj instead of verb бав|те.ся
#                 # вдумайт|еся # adj instead of verb вдума|йтеся
# }

# абсорбу|ватиму
# абсорбу|ватимеш
# абсорбу|ватиме
# абсорбу|ватимем
# абсорбу|ватимемо
# абсорбу|ватимете
# абсорбу|ватимуть
# абсорбу|вали
# абсорбо|вано

verb = {
  'ав али ало ала ать ати'  => %w[пізнав|ав пізнав|али пізнав|ало пізнав|ала пізнав|ать пізнав|ати],
  'іть'                     => %w[роб|іть],
  'йте'                     => %w[дума|йте],
  'ме'                      => %w[пізнавати|ме],
  'ла ло ли'                => %w[повез|ла     повез|ло  повез|ли],
  'ти ть те'                => %w[обігну|ти    поборо|ть обжені|те],
  'єш ємо єте ють'          => %w[чита|єш      чита|ємо  чита|єте   чита|ють],
  'еш емо ете уть'          => %w[пиш|еш       пиш|емо   пиш|ете    пиш|уть],
  'лю иш ить имо ите лять'  => %w[роб|лю       роб|иш    роб|ить    роб|имо   роб|ите   роб|лять],
  'ляти'                    => %w[вироб|ляти],
  'їш їть їмо їте ять яти'  => %w[сто|їш       сто|їть   сто|їмо    сто|їте   сто|ять   сто|яти],
}

noun = {
  'ам ами ах'       => %w[вод|ам      вод|ами   вод|ах],
  'ею'              => %w[пісн|ею],
  'ям ями ях'       => %w[пісн|ям     пісн|ями  пісн|ях],
  'ові ом'          => %w[батьк|ові   батьк|ом  бор|ом],
  'ець ем'          => %w[олів|ець    олівц|ем],
  'ень'             => %w[зел|ень],
  'ой'              => %w[гер|ой],
  'ію ії'           => %w[комед|ію    комед|ії],
  'ів'              => %w[вовчик|ів],
  'ев ов ей иям иях ию' => %w[дер|ев       електрол|ов солов|ей  кривош|иям  кривош|иях  кривош|ию],
  'іям іях'             => %w[промоакц|іям промоакц|іях],
  'ия єю еві єм їв'     => %w[кривош|ия    медіазбро|єю праотц|еві  праотц|ем промете|їв],
  'ією иєю еєю'         => %w[хім|ією      кривош|иєю   левз|еєю],
  'ьї ьє ьєю ью ья'     => %w[лазан|ьї     лазан|ьє     лазан|ьєю   лазан|ью  лазан|ья],
}

adj_verb_noun = {
  # adj
  'а е і у ю я'   => %w[зел|ен.а   зел|ен.е   зел|ен.і   зел|ен.у  верхн|ю  верхн|я],
  # verb
  'в е є й у ю'   => %w[нагляну|в     пиш|е      чита|є     чита|й    пиш|у    чита|ю],
  # noun
  'а е и і й '    => %w[вод|а      пісн|е     вод|и     вод|і     сара|й],
  'о у ь ю я'     => %w[вод|о      вод|у      ваган|ь   пісн|ю    пісн|я]
}

derivational = {
  'іст'       => %w[безтурботн|іст.ю незалежн|іст.ю],

  'ост'       => %w[безтурботн|ост.і   безтурботн|ост.ям  безтурботн|ост|ями
                    безтурботн|ост.ях  безтурботн|ост.ей  незалежн|ост.ями   їмост|ями ], # ??? ти/те/й

  'еньк'      => %w[посид|еньк.и холодн|еньк.ий]
}

need_to_approve  = {
  # noun
  'ві' =>  %w[район|о.ві], #? *ві
  'го' => %w[раду|го],
  'ка кою ку'                       => %w[єзуїт|ка        єзуїт|кою     єзуїт|ку
                                          сака            сакою         саку], # more then 2 syllables
  'очки очці очку очкою очка'       => %w[маків|очки маків|очці маків|очку маків|очкою маків|очка],
  'очко очок очкам очками очках'    => %w[маків|очко маків|очок маків|очкам маків|очками маків|очках],

  # verb
  'ала ало ась'                     => %w[поруб|ала        поруб|ало          докр|ась],
  'вав вавсь вався вала'            => %w[району|вав       району|вавсь       району|вався      району|вала],
  'валася вали вались валися вало'  => %w[району|валася    району|вали        району|валися     району|вало],
  'валось валося  ватись ватися'    => %w[району|валось    району|валося      району|ватись     району|ватися],
  'валась всь вся вати'             => %w[району|валась    зруши|всь          зруши|вся         району|вати],
  'есь еся'                         => %w[європеїзуйт|есь  європеїзуйт|еся],
  'ила или ило илося ити ись ися'   => %w[жар|ила          жар|или            жар|ило           жар|илося     жар|ити   жарит|ись жарит|ися],
  'ім імо'                          => %w[жаріюч|ім        жаріюч|імо],
  'ймо ймось ймося йсь йся'         => %w[жарту|ймо        жеврі|ймось        жеврі|ймося       єдна|йсь      єдна|йся],
  'лась лася  лись лися лось лося'  => %w[єдна|лась        єдна|лася          єдна|лись         єдна|лися     єдна|лось єдна|лося],
  'мо мось мося'                    => %w[жали|мо          жали|мось          жали|мося],
  'овував овувала овувати '         => %w[забальзам|овував забальзам|овувала  забальзам|овувати],
  'ось осте ості'                   => %w[пилос|ось        лукав|осте         лукав|ості        лушпин|очка],
  'сті сть стю'                     => %w[маневрено|сті    маневрені|сть      маневрені|стю],
  'ував увала увати'                => %w[маневр|ував      маневр|увала       маневр|увати],
}

tidy_up = {
  'дд'  => %w[усевлад|д.ям],
  'жж'  => %w[лівобереж|ж.ями],
  'лл'  => %w[весіл|л.ями],
  'нн'  => %w[осін|н.ій],
  'сс'  => %w[прус|с.ів],
  'тт'  => %w[століт|т.ями],
  'чч'  => %w[клоч|ч.я]
}

exceptions = {
  # ґ => г
  'аванґард'    => 'авангард',
  'аванґарду'   => 'авангард|у',
  'аванґардові' => 'авангард|ові',
  'аванґардом'  => 'авангард|ом',
  'аванґарді'   => 'авангард|і',
  'аванґарди'   => 'авангард|и',
  'аванґарде'   => 'авангард|е',
  'аванґардів'  => 'авангард|ів', # ?? в
  'аванґардам'  => 'авангард|ам',
  'аванґардами' => 'авангард|ами',
  'аванґардах'  => 'авангард|ах',

  # remove any apostrophe: ' ʹ ʻ ʼ ʽ ˈ ‘ ’ ‛ ′
  # "сім`я" => "сім", # cannot to remove Grave Accent
  "сім'я" => "сім",
  "сімʹя" => "сім",
  "сімʻя" => "сім",
  "сімʼя" => "сім",
  "сімʽя" => "сім",
  "сімˈя" => "сім",
  "сім‘я" => "сім",
  "сім’я" => "сім",
  "сім‛я" => "сім",
  "сім′я" => "сім",

  # Додатково перевірити слова:
  "брати" => "бра|ти",
  "беруть" => "бер|уть",

  "дописати" => "допис|ати",
  "допишуть" => "допиш|уть",

  "лікар" => "лікар",
  "лікарів" => "лікар|ів", # ??? в

  # прислівники:
  "добре"       => "добр|е",
  "добро"       => "добр|о",
  "часто"       => "част|о",
  "частіший"    => "част|іший",
  "довго"       => "довг|о",
  "довгий"      => "довг|ий",
  "чисто"       => "чист|о",
  "чистіший"    => "чист|іший",

  "робитимуть"  => "робитим|уть",

  'академію' => 'академ',
}


need_to_approve_2 = {
  # заміна
  'завойовував' => 'завойов', # завойов|ував
  'завойовувала' => 'завойов',
  'завойовувати' => 'завойов',

  'надкльовував' => 'надкльов', # надкльов|ував
  'надкльовувала' => 'надкльов',
  'надкльовувати' => 'надкльов',

  'витанцьовував' => 'витанцьов', # витанцьов|ував
  'витанцьовувала' => 'витанцьов',
  'витанцьовувати' => 'витанцьов',
}

need_to_approve_3 = {
  # для слів менше/рівне 3-м літерам не змінювати нічого
  'а' => 'а',
  'і' => 'і',
  'у' => 'у',
  'в' => 'в',
  'на' => 'на',
  'по' => 'по',
  'до' => 'до',
  'ні' => 'ні',
  'але' => 'але',
  'був' => 'був',
  'рік' => 'рік',
  'кіт' => 'кіт',
  'ніж' => 'ніж',
  'так' => 'так',

  # слова із 4-х літер - вдсікати останню літеру 'а в е є и і й о у ь ю я'
  'бува' => 'був',
  'віче' => 'віч',
  'наче' => 'нач',
  'паче' => 'пач',
  # року,
  # році,
  # роче,
  # роки,
  # ріка
  #   ріка, ріки, ріці, ріку, , ріці, ріко, ріки,  , ріки, , , ріки

  # для слів більше 4х літер усі правила
  # рокові,
  # роком,
  # років, рокам, роками, роках,
  # рікою рікам ріками ріках
  # річка, річку, річкою, річці, річко, річки, річок, річкам, річками, річках
  # рікша
  # відер
  # відра


  'неначе' => 'неначе',
  'одначе' => 'одначе',

  'пірʼя' => 'пір'
}

$all_tests = []
$errors = {}

require 'yaml'

def yml_h_tests(file)
  YAML.load_file(file)
end

def incorrect_stem_msg(result_stem, word, stem)
  # "Incorrect stemming '#{result_stem}' for word '#{word}', should be '#{stem}'"
  # "--------------------------------\n"
  "Word:      #{word}\n" +
  "should be: #{stem}\n" +
  "but it is: #{result_stem}\n"
end

def check_words_set(words_set, set_name)
  words_set.each do |_rule, test_cases|
    test_cases.each do |test_case|
      stem, ending = test_case.split('|')
      word = [stem, ending&.delete('.')].join
      $all_tests << word

      result_stem = (`echo "#{word}" | ../stemwords -l uk`).strip
      # if result_stem == 'ач'
      #   puts '>>'
      #   puts test_case
      #   puts stem
      #   puts ending
      #   puts word
      #   puts result_stem
      #   puts '<<'
      # end
      $errors[set_name] ||= []
      $errors[set_name] << incorrect_stem_msg(result_stem, word, stem) if result_stem != stem
    end
  end
end

[
  [perfective_gerund, 'perfective_gerund'],
  [adjective, 'adjective'],
  [adjectival, 'adjectival'],
  [reflexive, 'reflexive'],
  [verb, 'verb'],
  [noun, 'noun'],
  [adj_verb_noun, 'adj_verb_noun'],
  [derivational, 'derivational'],
  [tidy_up, 'tidy_up']
].each do |words_set, set_name|
  check_words_set(words_set, set_name)
end

[
  [exceptions, 'exceptions']
  # [yml_h_tests('./algorithms/uk_test_pairs_N1.yml'), 'uk_test_pairs_N1.yml']
].each do |words_set, set_name|
  words_set.each do |word, test_case|
    stem, ending = test_case.to_s.split('|')
    $all_tests << word
    result_stem = (`echo "#{word}" | ../stemwords -l uk `).strip
    $errors[set_name] ||= []
    $errors[set_name] << incorrect_stem_msg(result_stem, word, stem) if result_stem != stem
  end
end

if $errors.values.all?(&:empty?)
  puts("#{$all_tests.count} test(s) passed successfully!")
else
  $errors.each do |set_name, errs|
    next if errs.empty?

    puts "\n=== #{set_name}:\n\n"
    puts(errs.join("\n"))
  end
end

# a = %w[
#   ий ого ому им ім
#   іший ішого ішому ішим ішім
#    іше
#    ої
#   іша ішої ішій ішу ішою
#    их ими
#   іші іших ішими
#   ього ьому
#    ьої  ьою
#   іх іми
# ]

# v = %w[
#   ти
#   ть
#   іть
#   те
#   йте
#   ать
#   ла ло ли
#    єш  ємо єте ють
#    еш  емо ете уть
#   лю иш ить имо ите лять
#   їш їть їмо їте ять
# ]

# n = %w[
#   ам ами ах
#   ею
#   ям ями ях
#   ові ом
#   ець ем
#   ень
#   ой
#   ію ії
# ]

# adj_verb_noun = %w[
#   е а у  і я ю
#   й в ю є у е
#   а и і у о я ю е й ь
#   ою ій
# ]

# puts "a & v"
# p (a & v).sort

# puts "v & n"
# p (v & n).sort

# puts "n & a"
# p (n & a).sort

# ToDo:

# +1. Ґ => Г
#     do repeat ( goto (['ґ]) <- 'г' )                                      // угнетённый => угнетен

# +2. Видалити всі типи Апострофів 'ʼ`

# +3. Додатково перевірити слова:



# Add yml tests
# https://github.com/Tapkomet/UAStemming/blob/master/Results.txt
