#include "parser.h"
#include <sstream>

//Выполняем синтаксический разбор блока program. Если во время разбора не обнаруживаем 
//никаких ошибок, то выводим последовательность команд стек-машины
void Parser::parse()
{
	program(); 
	if(!error_) {
		codegen_->flush();
	}
}

void Parser::program()
{
	mustBe(T_BEGIN);
	statementList();
	mustBe(T_END);
	codegen_->emit(STOP);
}

/**
 * 匹配结尾运算符：
 *  - END
 *  - OD
 *  - ELSE
 *  - FI
 */
void Parser::statementList()
{
	//	Если список операторов пуст, очередной лексемой будет одна из возможных "закрывающих скобок": END, OD, ELSE, FI.
	//	В этом случае результатом разбора будет пустой блок (его список операторов равен null).
	//	Если очередная лексема не входит в этот список, то ее мы считаем началом оператора и вызываем метод statement.
	//  Признаком последнего оператора является отсутствие после оператора точки с запятой.
    //  如果运算符列表为空，下一个标记将是可能的 "结束括号 "之一：END、OD、ELSE、FI。
    //  在这种情况下，解析的结果将是一个空块（其运算符列表为 null）。
    //  如果下一个标记不在这个列表中，我们认为它是一个运算符的开始，并调用语句方法。
    //  【重点】最后一个声明的标志是运算符后面没有分号。
	if(see(T_END) || see(T_OD) || see(T_ELSE) || see(T_FI)) {
		return;
	}
	else {
		bool more = true;
		while(more) {
			statement();
			more = match(T_SEMICOLON);  // `;`
		}
	}
}

/**
 * 运算符分析：
 *  - 变量
 *  - IF
 *  - WHILE
 *  - WRITE
 */
void Parser::statement()
{
    /*
	 * Если встречаем переменную, то запоминаем ее адрес или добавляем новую если не встретили.
	 * Следующей лексемой должно быть присваивание. Затем идет блок expression, который возвращает значение на вершину стека.
	 * Записываем это значение по адресу нашей переменной
     * 如果我们遇到一个变量，我们会记住它的地址，如果没有的话，就添加一个新的地址。
     * 下一个词素应该是一个任务。然后是表达式块，它向栈顶返回一个值。
     * 把这个值写到我们的变量地址中
     * */
	if(see(T_IDENTIFIER)) {
		int varAddress = findOrAddVariable(scanner_->getStringValue());
		next();
		mustBe(T_ASSIGN);  // `:=`
		expression();
		codegen_->emit(STORE, varAddress);
	}

    /*
	 * Если встретили IF, то затем должно следовать условие. На вершине стека лежит 1 или 0 в зависимости от выполнения условия.
	 * Затем зарезервируем место для условного перехода JUMP_NO к блоку ELSE (переход в случае ложного условия). Адрес перехода
	 * станет известным только после того, как будет сгенерирован код для блока THEN.
     * 如果遇到了IF，那么条件必须随之而来。在堆栈的顶部是1或0，取决于是否满足条件。
     * 然后为条件性过渡JUMP_NO到ELSE块保留空间（在错误条件下的过渡）。跳转地址
     * 只有在THEN块的代码生成后才知道。
     * */
	else if(match(T_IF)) {
		relation();  // 用于比较表达式，结果以 0 或 1 放置在栈的顶端

        // 生成一个空的地址
		int jumpNoAddress = codegen_->reserve();

        // 在 IF 后面必须是运算符 THEN
		mustBe(T_THEN);
        // 判断 THEN 之后的运算符是不是 END、OD、ELSE、FI 之一的
		statementList();

		if(match(T_ELSE)) {  // 如果匹配到了 ELSE

            // Если есть блок ELSE, то чтобы не выполнять его в случае выполнения THEN, зарезервируем место для команды JUMP в конец этого блока
            // 如果有一个 ELSE 块，为了在 THEN 被执行时不执行它，我们在这个块的末尾为 JUMP 命令保留空间
			int jumpAddress = codegen_->reserve();

            // Заполним зарезервированное место после проверки условия инструкцией перехода в начало блока ELSE.
            // 如果 IF 后面的条件为【JUMP_NO：0】，则将之前保留的空间（jumpNoAddress），置为跳转到 ELSE
			codegen_->emitAt(jumpNoAddress, JUMP_NO, codegen_->getCurrentAddress());
			statementList();

            // Заполним второй адрес инструкцией перехода в конец условного блока ELSE.
            // 在ELSE条件块的末尾，用过渡指令填入第二个地址。
			codegen_->emitAt(jumpAddress, JUMP, codegen_->getCurrentAddress());
		}
		else {  // 如果当前IF 语句里没有 ELSE 语句
            //Если блок ELSE отсутствует, то в зарезервированный адрес после проверки условия будет записана
            //инструкция условного перехода в конец оператора IF...THEN
            // 如果没有ELSE块，在检查条件后，一条条件跳转指令将被写入IF...THEN语句末尾的保留地址中
			codegen_->emitAt(jumpNoAddress, JUMP_NO, codegen_->getCurrentAddress());
		}

		mustBe(T_FI);
	}

	else if(match(T_WHILE)) {
		// Запоминаем адрес начала проверки условия.
        // 条件检查的起始地址
		int conditionAddress = codegen_->getCurrentAddress();
        this->whileContinueAddress_ = conditionAddress;
		relation();  // 条件解析

		// Резервируем место под инструкцию условного перехода для выхода из цикла.
        // 为退出 WHILE 循环的条件性跳转指令保留空间
		int jumpNoAddress = codegen_->reserve();
        this->whileBreakAddress_ = jumpNoAddress;
		mustBe(T_DO);
		statementList();
        mustBe(T_OD);

		// Переходим по адресу проверки условия
        // 转到条件检查地址，其中 JUMP 为无条件跳转
		codegen_->emit(JUMP, conditionAddress);

		// Заполняем зарезервированный адрес инструкцией условного перехода на следующий за циклом оператор.
        // 如果不满足条件即跳出循环：用循环后的运算符上的条件跳转指令填入保留地址。
		codegen_->emitAt(jumpNoAddress, JUMP_NO, codegen_->getCurrentAddress());
	}
#if 1
    /** - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * 实现 `BREAK` 的思想：
     *  `BREAK` 的效果实际上和 `WHILE` 循环的条件不成立的效果一致，所以根据虚拟机的原理将实现 `BREAK` 分为了 3 步：
     *      1. 向栈中（当前语句后方） PUSH 一个 0，以作为 COMPARE 为 FALSE 的结果（Лекция 5, стр 10）
     *      2. 紧接着向栈中增加无条件跳转语句 JUMP，跳转至 WHILE 循环判断不成立处（JUMP_NO）
     *      3. 继续向后进行词素分析
     *
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * 写在 отчет 里的：
     * 实现运算符 `BREAK` 的思想：
     *   运算符 `BREAK` 的效果实际上和 `WHILE` 循环的条件不成立的效果一致，所以根据虚拟机的原理将实现 `BREAK` 分为了 6 步：
     *      1. 在运算符列表中增加 BREAK
     *      2. 增加存储 WHILE 语句开始（条件判断处之后那条语句）及结束（退出）地址的暂存区的
     *      3. 在检测到 WHILE 循环时将其开始和结束的地址保存
     *      4. 在检测到 BREAK 且 WHILE 也存在时，向栈中（当前语句后方） PUSH 一个 0，以作为 COMPARE 为 FALSE 的结果，否则报错
     *      5. 紧接着向栈中增加无条件跳转语句 JUMP，跳转至 WHILE 循环判断不成立处（JUMP_NO）
     *      6. 继续向后进行词素分析
     *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     */
    else if (match(T_BREAK))
    {
        if (-1 == this->whileBreakAddress_)
        {
            reportError("[ERROR] No `WHILE` match found for `BREAK`");
            return;
        }
        // 1. 向栈中（当前语句后方） PUSH 一个 0，以作为 COMPARE 为 FALSE 的结果（Лекция 5, стр 10）
        int cmpFalse = codegen_->reserve();
        codegen_->emitAt(cmpFalse, PUSH, 0);

        // 2. 紧接着向栈中增加无条件跳转语句 JUMP，跳转至 WHILE 循环判断不成立处（JUMP_NO）
        int jumpAddress = codegen_->reserve();
        codegen_->emitAt(jumpAddress, JUMP, this->whileBreakAddress_);

        // 3. 继续向后进行语句分析
        statementList();
    }
#endif

#if 1
    /**
    * 实现 `CONTINUE` 的思想：
    *    `CONTINUE` 的效果为跳转到 WHILE 循环条件判断处，所以根据虚拟机的原理将实现 `CONTINUE` 分为了 2 步：
    *      1. 向栈中（当前语句后方） PUSH 一条无条件跳转指令 JUMP，跳转至 WHILE 开始处，重新进行条件判断
    *      2. 继续向后进行词素分析
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * 写在 отчет 里的：
     * 实现运算符 `CONTINUE` 的思想：
     *   运算符 `CONTINUE` 的效果为跳转到 WHILE 循环条件判断处，所以根据虚拟机的原理将实现 `CONTINUE` 分为了 5 步：
     *      1. 在运算符列表中增加 CONTINUE
     *      2. 增加存储 WHILE 语句开始（条件判断处之后那条语句）及结束（退出）地址的暂存区的
     *      3. 在检测到 WHILE 循环时将其开始和结束的地址保存
     *      4. 在检测到 CONTINUE 且 WHILE 也存在时，向栈中（当前语句后方） PUSH 一条无条件跳转指令 JUMP，以跳转至 WHILE 开始处，重新进行条件判断
     *      5. 继续向后进行词素分析
     *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    */
    else if (match(T_CONTINUE))
    {
        if (-1 == this->whileContinueAddress_)
        {
            reportError("[ERROR] No `WHILE` match found for `CONTINUE`");
            return;
        }

        // 1. 向栈中（当前语句后方） PUSH 一条无条件跳转指令 JUMP，跳转至 WHILE 开始处，重新进行条件判断
        int jumpAddress = codegen_->reserve();
        codegen_->emitAt(jumpAddress, JUMP, this->whileContinueAddress_);

        // 2. 继续向后进行语句分析
        statementList();
    }
#endif

	else if(match(T_WRITE)) {
		mustBe(T_LPAREN);
		expression();
		mustBe(T_RPAREN);
		codegen_->emit(PRINT);
	}
	else {
		reportError("[ERROR] Statement expected.");
	}
}

/**
 * 解析算数表达式 —— 加减运算
 */
void Parser::expression()
{

	 /*
         Арифметическое выражение описывается следующими правилами: <expression> -> <term> | <term> + <term> | <term> - <term>
         При разборе сначала смотрим первый терм, затем анализируем очередной символ. Если это '+' или '-', 
		 удаляем его из потока и разбираем очередное слагаемое (вычитаемое). Повторяем проверку и разбор очередного 
		 терма, пока не встретим за термом символ, отличный от '+' и '-'

	    一个算术表达式由以下规则描述：<表达式>-><term> | <term> + <term> | <term> - <term>。
        解析时，我们先看第一个词，然后再分析下一个字符。如果是'+'或'-'。
        我们将其从线程中移除，并解析下一个和值。重复检查和解析下一个术语
        直到我们找到一个除'+'和'-'以外的字符为止。
     */

	term();
	while(see(T_ADDOP)) {
		Arithmetic op = scanner_->getArithmeticValue();
		next();
		term();

		if(op == A_PLUS) {
			codegen_->emit(ADD);
		}
		else {
			codegen_->emit(SUB);
		}
	}
}

/**
 * 解析算数表达式 —— 乘除运算
 */
void Parser::term()
{
	 /*  
		 Терм описывается следующими правилами: <expression> -> <factor> | <factor> + <factor> | <factor> - <factor>
         При разборе сначала смотрим первый множитель, затем анализируем очередной символ. Если это '*' или '/', 
		 удаляем его из потока и разбираем очередное слагаемое (вычитаемое). Повторяем проверку и разбор очередного 
		 множителя, пока не встретим за ним символ, отличный от '*' и '/' 
	*/
	factor();
	while(see(T_MULOP)) {
		Arithmetic op = scanner_->getArithmeticValue();
		next();
		factor();

		if(op == A_MULTIPLY) {
			codegen_->emit(MULT);
		}
		else {
			codegen_->emit(DIV);
		}
	}
}

/**
 * 解析算数表达式 —— 乘法分析
 */
void Parser::factor()
{
	/*
		Множитель описывается следующими правилами:
		<factor> -> number | identifier | -<factor> | (<expression>) | READ
	*/
	if(see(T_NUMBER)) {
		int value = scanner_->getIntValue();
		next();
		codegen_->emit(PUSH, value);
		//Если встретили число, то преобразуем его в целое и записываем на вершину стека
	}
	else if(see(T_IDENTIFIER)) {
		int varAddress = findOrAddVariable(scanner_->getStringValue());
		next();
		codegen_->emit(LOAD, varAddress);
		//Если встретили переменную, то выгружаем значение, лежащее по ее адресу, на вершину стека 
	}
	else if(see(T_ADDOP) && scanner_->getArithmeticValue() == A_MINUS) {
		next();
		factor();
		codegen_->emit(INVERT);
		//Если встретили знак "-", и за ним <factor> то инвертируем значение, лежащее на вершине стека
	}
	else if(match(T_LPAREN)) {
		expression();
		mustBe(T_RPAREN);
		//Если встретили открывающую скобку, тогда следом может идти любое арифметическое выражение и обязательно
		//закрывающая скобка.
	}
	else if(match(T_READ)) {
		codegen_->emit(INPUT);
		//Если встретили зарезервированное слово READ, то записываем на вершину стека идет запись со стандартного ввода
	}
	else {
		reportError("expression expected.");
	}
}

/**
 * 条件解析
 */
void Parser::relation()
{
	//Условие сравнивает два выражения по какому-либо из знаков. Каждый знак имеет свой номер. В зависимости от 
	//результата сравнения на вершине стека окажется 0 или 1.
	expression();
	if(see(T_CMP)) {
		Cmp cmp = scanner_->getCmpValue();
		next();
		expression();
		switch(cmp) {
			//для знака "=" - номер 0
			case C_EQ:
				codegen_->emit(COMPARE, 0);
				break;
			//для знака "!=" - номер 1
			case C_NE:
				codegen_->emit(COMPARE, 1);
				break;
			//для знака "<" - номер 2
			case C_LT:
				codegen_->emit(COMPARE, 2);
				break;
			//для знака ">" - номер 3
			case C_GT:
				codegen_->emit(COMPARE, 3);
				break;
			//для знака "<=" - номер 4
			case C_LE:
				codegen_->emit(COMPARE, 4);
				break;
			//для знака ">=" - номер 5
			case C_GE:
				codegen_->emit(COMPARE, 5);
				break;
		};
	}
	else {
		reportError("[ERROR] Comparison operator expected.");
	}
}


int Parser::findOrAddVariable(const string& var)
{
	VarTable::iterator it = variables_.find(var);
	if(it == variables_.end()) {
		variables_[var] = lastVar_;
		return lastVar_++;
	}
	else {
		return it->second;
	}
}

void Parser::mustBe(Token t)
{
	if(!match(t)) {
		error_ = true;

		// Подготовим сообщение об ошибке
		std::ostringstream msg;
		msg << tokenToString(scanner_->token()) << " found while " << tokenToString(t) << " expected.";
		reportError(msg.str());

		// Попытка восстановления после ошибки.
		recover(t);
	}
}

void Parser::recover(Token t)
{
	while(!see(t) && !see(T_EOF)) {
		next();
	}

	if(see(t)) {
		next();
	}
}
