package visitor;

import ast.*;
import ast.Function;
import ast.type.*;
import org.antlr.v4.runtime.tree.*;
import parser.CParser;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/24/16.
 */
public class TreeToASTVisitor {

    public static final boolean GROUP_EXPRESSIONS = true;
    public static final boolean LOOPS_AND_IFS_MUST_BE_COMPOUND_STATEMENTS = true;
    public static final boolean ELSE_IF_CHAIN_WITHOUT_COMPOUND_STATEMENTS = true;

    public Program visit(CParser.CompilationUnitContext ctx) {
        Collection<Function> functions = new ArrayList<>();
        Collection<Declaration> declarations = new ArrayList<>();
        CParser.TranslationUnitContext translationUnitContext = ctx.translationUnit();
        List<CParser.ExternalDeclarationContext> externalDeclarationContexts = translationUnitContext.externalDeclaration();
        for (CParser.ExternalDeclarationContext externalDeclarationContext : externalDeclarationContexts) {
            if (externalDeclarationContext.functionDefinition() != null) {
                Function function = visit(externalDeclarationContext.functionDefinition());
                functions.add(function);
            } else if (externalDeclarationContext.declaration() != null) {
                Declaration declaration = visit(externalDeclarationContext.declaration());
                declarations.add(declaration);
            }
        }
        return new Program(functions, declarations);
    }

    private String visit(CParser.DeclaratorContext ctx) {
        CParser.DirectDeclaratorContext currentContext = ctx.directDeclarator();
        while (currentContext.Identifier() == null) {
            currentContext = currentContext.directDeclarator();
        }
        return currentContext.Identifier().getSymbol().getText();
    }

    private Function visit(CParser.FunctionDefinitionContext ctx) {
        Type type = getType(ctx.declarationSpecifiers(), ctx.declarator());
        String identifier = visit(ctx.declarator());
        ParameterList parameterList = visit(ctx.declarator().directDeclarator().parameterTypeList());
        CompoundStatement compoundStatement = visit(ctx.compoundStatement());
        return new Function(type, identifier, parameterList, compoundStatement);
    }

    private Type visit(CParser.DeclarationSpecifiersContext ctx) {
        return visit(ctx.typeSpecifier());
    }

    private ParameterList visit(CParser.ParameterTypeListContext ctx) {
        if (ctx != null) {
            return visit(ctx.parameterList());
        } else {
            return new ParameterList();
        }
    }

    private ParameterList visit(CParser.ParameterListContext ctx) {
        List<CParser.ParameterDeclarationContext> parameterDeclarationContexts = ctx.parameterDeclaration();
        List<Parameter> parameters = parameterDeclarationContexts.stream().map(para -> visit(para)).collect(Collectors.toList());
        return new ParameterList(parameters);
    }

    private Parameter visit(CParser.ParameterDeclarationContext ctx) {
        Type type = getType(ctx.declarationSpecifiers(), ctx.declarator());
        String identifier = visit(ctx.declarator());
        return new Parameter(type, identifier);
    }

    private CompoundStatement visit(CParser.CompoundStatementContext ctx) {
        if (ctx.blockItemList() != null) {
            List<BlockItem> blockItems = visit(ctx.blockItemList());
            return new CompoundStatement(blockItems);
        } else {
            return new CompoundStatement(Collections.emptyList());
        }
    }

    private List<BlockItem> visit(CParser.BlockItemListContext ctx) {
        if (ctx.blockItem() != null) {
            List<CParser.BlockItemContext> blockItemContexts = ctx.blockItem();
            return blockItemContexts.stream().map(bl -> visit(bl)).collect(Collectors.toList());
        } else {
            return Collections.emptyList();
        }
    }

    private BlockItem visit(CParser.BlockItemContext ctx) {
        if (ctx.declaration() != null && ctx.statement() != null) {
            System.err.println("BOTH NOT NULL");
        }
        if (ctx.declaration() != null) {
            return visit(ctx.declaration());
        } else {
            return visit(ctx.statement());
        }
    }

    private Declaration visit(CParser.DeclarationContext ctx) {
        Type type = visit(ctx.declarationSpecifiers());
        CParser.InitDeclaratorListContext initDeclaratorListContext = ctx.initDeclaratorList();
        List<CParser.InitDeclaratorContext> initDeclaratorContexts = initDeclaratorListContext.initDeclarator();
        List<Declaration.DeclaredVariable> declaredVariables = initDeclaratorContexts.stream().map(init -> visit(init, type)).collect(Collectors.toList());
        return new Declaration(declaredVariables);
    }

    private Declaration.DeclaredVariable visit(CParser.InitDeclaratorContext ctx, Type type) {
        String identifier = visit(ctx.declarator());
        if (ctx.declarator().pointer() != null) {
            int nestedPointers = visit(ctx.declarator().pointer());
            type = new PointerType(nestedPointers, (ActualType)type);
        }
        if (ctx.initializer() != null) {
            CParser.InitializerContext initializerContext = ctx.initializer();
            AssignmentExpression assignmentExpression = visit(initializerContext);
            return new Declaration.DeclaredVariable(type, identifier, assignmentExpression);
        } else {
            return new Declaration.DeclaredVariable(type, identifier);
        }
    }

    private AssignmentExpression visit(CParser.InitializerContext ctx) {
        if (ctx.assignmentExpression() != null) {
            return visit(ctx.assignmentExpression());
        } else {
            List<AssignmentExpression> assignmentExpressions = new ArrayList<>();
            CParser.InitializerListContext initializerListContext = ctx.initializerList();
            while (initializerListContext != null) {
                assignmentExpressions.add(visit(initializerListContext.initializer()));
                initializerListContext = initializerListContext.initializerList();
            }
            return new InitializerList(assignmentExpressions);
        }
    }

    private Statement visit(CParser.StatementContext ctx) {
        if (ctx.labeledStatement() != null) {
            return visit(ctx.labeledStatement());
        } else if (ctx.compoundStatement() != null) {
            return visit(ctx.compoundStatement());
        } else if (ctx.expressionStatement() != null) {
            return visit(ctx.expressionStatement());
        } else if (ctx.selectionStatement() != null) {
            return visit(ctx.selectionStatement());
        } else if (ctx.iterationStatement() != null) {
            return visit(ctx.iterationStatement());
        } else if (ctx.jumpStatement() != null) {
            return visit(ctx.jumpStatement());
        }
        throw new UnsupportedOperationException("What kind of statement is this?");
    }

    private LabeledStatement visit(CParser.LabeledStatementContext ctx) {
        Statement statement = visit(ctx.statement());
        if (ctx.Identifier() != null) {
            return new LabeledIdentifierStatement(ctx.Identifier().getSymbol().getText(), statement);
        } else if (ctx.constantExpression() != null) {
            return new LabeledCaseStatement(visit(ctx.constantExpression().conditionalExpression()), statement);
        } else {
            return new LabeledDefaultStatement(statement);
        }
    }

    private ExpressionStatement visit(CParser.ExpressionStatementContext ctx) {
        if (ctx.expression() != null) {
            Expression expression = visit(ctx.expression());
            return new ExpressionStatement(expression);
        }
        throw new IllegalArgumentException("ExpressionStatement returning null");
    }

    private SelectionStatement visit(CParser.SelectionStatementContext ctx) {
        if (ctx.ifStatement() != null) {
            CParser.IfStatementContext ifStatementContext = ctx.ifStatement();
            Expression condition = visit(ifStatementContext.expression());
            List<CParser.StatementContext> statementContexts = ifStatementContext.statement();
            Statement thenStatement = visit(statementContexts.get(0));
            Statement elseStatement = null;
            if (statementContexts.size() > 1) {
                elseStatement = visit(statementContexts.get(1));
                if (ELSE_IF_CHAIN_WITHOUT_COMPOUND_STATEMENTS && !(elseStatement instanceof SelectionStatementIf)) {
                    if (LOOPS_AND_IFS_MUST_BE_COMPOUND_STATEMENTS && !(elseStatement instanceof CompoundStatement)) {
                        elseStatement = containInCompoundStatement(elseStatement);
                    }
                }
            }

            if (LOOPS_AND_IFS_MUST_BE_COMPOUND_STATEMENTS && !(thenStatement instanceof CompoundStatement)) {
                thenStatement = containInCompoundStatement(thenStatement);
            }

            return new SelectionStatementIf(condition, thenStatement, elseStatement);
        } else {
            CParser.SwitchStatementContext switchStatementContext = ctx.switchStatement();
            Expression expression = visit(switchStatementContext.expression());
            Statement statement = visit(switchStatementContext.statement());
            return new SelectionStatementSwitch(expression, statement);
        }
    }

    private IterationStatement visit(CParser.IterationStatementContext ctx) {
        if (ctx.forLoopStatement() != null) {
            return visit(ctx.forLoopStatement());
        } else if (ctx.doWhileStatement() != null) {
            return visit(ctx.doWhileStatement());
        } else if (ctx.whileStatement() != null) {
            return visit(ctx.whileStatement());
        } else {
            return visit(ctx.declareForLoopStatement());
        }
    }

    private CompoundStatement containInCompoundStatement(Statement statement) {
        if (statement == null) {
            return null;
        }
        List<BlockItem> blockItems = new ArrayList<>();
        blockItems.add(statement);
        return new CompoundStatement(blockItems);
    }

    private IterationStatementFor visit(CParser.ForLoopStatementContext ctx) {
        Expression initExpression = null, condExpression = null, iterExpression = null;
        if (ctx.initExpression() != null) {
            initExpression = visit(ctx.initExpression().expression());
        }
        if (ctx.condExpression() != null) {
            condExpression = visit(ctx.condExpression().expression());
        }
        if (ctx.iterExpression() != null) {
            iterExpression = visit(ctx.iterExpression().expression());
        }
        Statement statement = visit(ctx.statement());
        if (LOOPS_AND_IFS_MUST_BE_COMPOUND_STATEMENTS && !(statement instanceof CompoundStatement)) {
            statement = containInCompoundStatement(statement);
        }
        return new IterationStatementFor(initExpression, condExpression, iterExpression, statement);
    }

    private IterationStatementDoWhile visit(CParser.DoWhileStatementContext ctx) {
        Statement statement = visit(ctx.statement());
        Expression expression = visit(ctx.expression());
        if (LOOPS_AND_IFS_MUST_BE_COMPOUND_STATEMENTS && !(statement instanceof CompoundStatement)) {
            statement = containInCompoundStatement(statement);
        }
        return new IterationStatementDoWhile(statement, expression);
    }

    private IterationStatementWhile visit(CParser.WhileStatementContext ctx) {
        Expression expression = visit(ctx.expression());
        Statement statement = visit(ctx.statement());
        if (LOOPS_AND_IFS_MUST_BE_COMPOUND_STATEMENTS && !(statement instanceof CompoundStatement)) {
            statement = containInCompoundStatement(statement);
        }
        return new IterationStatementWhile(expression, statement);
    }

    private IterationStatementDeclareFor visit(CParser.DeclareForLoopStatementContext ctx) {
        Declaration declaration = visit(ctx.declaration());
        Expression expression = null, iteration = null;
        if (ctx.condExpression() != null) {
            expression = visit(ctx.condExpression().expression());
        }
        if (ctx.iterExpression() != null) {
            iteration = visit(ctx.iterExpression().expression());
        }
        Statement statement = visit(ctx.statement());
        if (LOOPS_AND_IFS_MUST_BE_COMPOUND_STATEMENTS && !(statement instanceof CompoundStatement)) {
            statement = containInCompoundStatement(statement);
        }
        return new IterationStatementDeclareFor(declaration, expression, iteration, statement);
    }

    private JumpStatement visit(CParser.JumpStatementContext ctx) {
        List<ParseTree> children = ctx.children;
        switch (children.get(0).getText()) {
            case "continue":
                return new JumpContinueStatement();
            case "break":
                return new JumpBreakStatement();
            case "return":
                return new JumpReturnStatement(visit(ctx.expression()));
            default:
                throw new UnsupportedOperationException("What kind of jump statement is this? " + ctx);
        }
    }

    private Expression visit(CParser.ExpressionContext ctx) {
        List<CParser.AssignmentExpressionContext> assignmentExpressionContexts = ctx.assignmentExpression();
        List<AssignmentExpression> assignmentExpressions = assignmentExpressionContexts.stream().map(exp -> visit(exp)).collect(Collectors.toList());
        if (assignmentExpressions.size() > 1) {
            return new CommaExpression(assignmentExpressions);
        } else {
            return assignmentExpressions.get(0);
        }
    }

    private AssignmentExpression visit(CParser.AssignmentExpressionContext ctx) {
        if (ctx.conditionalExpression() != null) {
            return visit(ctx.conditionalExpression());
        } else {
            UnaryExpression unaryExpression = visit(ctx.unaryExpression());
            AssignmentExpressionImpl.AssignmentOperator assignmentOperator = AssignmentExpressionImpl.AssignmentOperator.toAssignmentOperator(ctx.assignmentOperator().getText());
            AssignmentExpression assignmentExpression = visit(ctx.assignmentExpression());
            return new AssignmentExpressionImpl(unaryExpression, assignmentOperator, assignmentExpression);
        }
    }

    private ConditionalExpression visit(CParser.ConditionalExpressionContext ctx) {
        LogicalOrExpression logicalOrExpression = visit(ctx.logicalOrExpression());
        if (ctx.expression() == null) {
            return logicalOrExpression;
        } else {
            Expression thenExpression = visit(ctx.expression());
            ConditionalExpression elseExpression = visit(ctx.conditionalExpression());
            return new ConditionalExpressionImpl(logicalOrExpression, thenExpression, elseExpression);
        }
    }

    private LogicalOrExpression visit(CParser.LogicalOrExpressionContext ctx) {
        List<LogicalAndExpression> logicalAndExpressions = ctx.logicalAndExpression().stream().map(exp -> visit(exp)).collect(Collectors.toList());
        if (logicalAndExpressions.size() == 1) {
            return logicalAndExpressions.get(0);
        } else {
            return new LogicalOrExpressionImpl(logicalAndExpressions);
        }
    }

    private LogicalAndExpression visit(CParser.LogicalAndExpressionContext ctx) {
        List<InclusiveBitwiseOrExpression> inclusiveBitwiseOrExpressions = ctx.inclusiveOrExpression().stream().map(exp -> visit(exp)).collect(Collectors.toList());
        if (inclusiveBitwiseOrExpressions.size() > 1) {
            return new LogicalAndExpressionImpl(inclusiveBitwiseOrExpressions);
        } else {
            return inclusiveBitwiseOrExpressions.get(0);
        }
    }

    private InclusiveBitwiseOrExpression visit(CParser.InclusiveOrExpressionContext ctx) {
        List<ExclusiveBitwiseOrExpression> exclusiveBitwiseOrExpressions = ctx.exclusiveOrExpression().stream().map(exp -> visit(exp)).collect(Collectors.toList());
        if (exclusiveBitwiseOrExpressions.size() > 1) {
            return new InclusiveBitwiseOrExpressionImpl(exclusiveBitwiseOrExpressions);
        } else {
            return exclusiveBitwiseOrExpressions.get(0);
        }
    }

    private ExclusiveBitwiseOrExpression visit(CParser.ExclusiveOrExpressionContext ctx) {
        List<BitwiseAndExpression> bitwiseAndExpressions = ctx.andExpression().stream().map(exp -> visit(exp)).collect(Collectors.toList());
        if (bitwiseAndExpressions.size() == 1) {
            return bitwiseAndExpressions.get(0);
        } else {
            return new ExclusiveBitwiseOrExpressionImpl(bitwiseAndExpressions);
        }
    }

    private BitwiseAndExpression visit(CParser.AndExpressionContext ctx) {
        List<EqualityExpression> equalityExpressions = ctx.equalityExpression().stream().map(exp -> visit(exp)).collect(Collectors.toList());
        if (equalityExpressions.size() == 1) {
            return equalityExpressions.get(0);
        } else {
            return new BitwiseAndExpressionImpl(equalityExpressions);
        }
    }

    private EqualityExpression visit(CParser.EqualityExpressionContext ctx) {
        RelationalExpression relationalExpression = visit(ctx.relationalExpression());

        if (ctx.equalityExpression() != null) {
            EqualityExpressionImpl.EqualityOperator equalityOperator = EqualityExpressionImpl.EqualityOperator.toEqualityOperator(ctx.getChild(1).getText());
            EqualityExpression equalityExpression = visit(ctx.equalityExpression());
            return new EqualityExpressionImpl(equalityExpression, equalityOperator, relationalExpression);
        } else {
            return relationalExpression;
        }
    }

    private RelationalExpression visit(CParser.RelationalExpressionContext ctx) {
        BitwiseShiftExpression bitwiseShiftExpression = visit(ctx.shiftExpression());
        if (ctx.relationalExpression() != null) {
            RelationalExpressionImpl.RelationalOperator relationalOperator = RelationalExpressionImpl.RelationalOperator.toRelationalOperator(ctx.getChild(1).getText());
            RelationalExpression relationalExpression = visit(ctx.relationalExpression());
            return new RelationalExpressionImpl(relationalExpression, relationalOperator, bitwiseShiftExpression);
        } else {
            return bitwiseShiftExpression;
        }
    }

    private BitwiseShiftExpression visit(CParser.ShiftExpressionContext ctx) {
        AdditiveExpression additiveExpression = visit(ctx.additiveExpression());
        if (ctx.shiftExpression() != null) {
            BitwiseShiftExpression bitwiseShiftExpression = visit(ctx.shiftExpression());
            BitwiseShiftExpressionImpl.ShiftOperator shiftOperator = BitwiseShiftExpressionImpl.ShiftOperator.toShiftOperator(ctx.getChild(1).getText());
            return new BitwiseShiftExpressionImpl(bitwiseShiftExpression, shiftOperator, additiveExpression);
        } else {
            return additiveExpression;
        }

    }

    private AdditiveExpression visit(CParser.AdditiveExpressionContext ctx) {
        MultiplicativeExpression multiplicativeExpression = visit(ctx.multiplicativeExpression());
        if (ctx.additiveExpression() != null) {
            if (GROUP_EXPRESSIONS) {
                return visitAddGroups(ctx);
            } else {
                AdditiveExpression additiveExpression = visit(ctx.additiveExpression());
                AdditiveExpression.AdditiveOperator additiveOperator = AdditiveExpression.AdditiveOperator.toAdditiveOperator(ctx.getChild(1).getText());
                return new AdditiveExpressionImplTree(additiveExpression, additiveOperator, multiplicativeExpression);
            }
        } else {
            return multiplicativeExpression;
        }
    }

    private AdditiveExpressionImplGroups visitAddGroups(CParser.AdditiveExpressionContext ctx) {
        Collection<MultiplicativeExpression> addingTerms = new ArrayList<>();
        Collection<MultiplicativeExpression> subtractingTerms = new ArrayList<>();

        while (ctx.additiveExpression() != null) {
            MultiplicativeExpression multiplicativeExpression = visit(ctx.multiplicativeExpression());
            AdditiveExpression.AdditiveOperator additiveOperator = AdditiveExpression.AdditiveOperator.toAdditiveOperator(ctx.getChild(1).getText());
            switch (additiveOperator) {
                case PLUS:
                    addingTerms.add(multiplicativeExpression);
                    break;
                case MINUS:
                    subtractingTerms.add(multiplicativeExpression);
                    break;
                default:
                    System.err.println(ctx.getChild(1).getText());
                    System.err.println("NOT PLUS OR MINUS?");
                    System.exit(0);
            }
            ctx = ctx.additiveExpression();
        }
        addingTerms.add(visit(ctx.multiplicativeExpression()));
        return new AdditiveExpressionImplGroups(addingTerms, subtractingTerms);
    }

    private MultiplicativeExpression visit(CParser.MultiplicativeExpressionContext ctx) {
        CastExpression castExpression = visit(ctx.castExpression());
        if (ctx.multiplicativeExpression() != null) {
            MultiplicativeExpression multiplicativeExpression = visit(ctx.multiplicativeExpression());
            MultiplicativeExpressionImpl.MultiplicativeOperator multiplicativeOperator = MultiplicativeExpressionImpl.MultiplicativeOperator.toMultiplicativeOperator(ctx.getChild(1).getText());
            return new MultiplicativeExpressionImpl(multiplicativeExpression, multiplicativeOperator, castExpression);
        } else {
            return castExpression;
        }
    }

    private CastExpression visit(CParser.CastExpressionContext ctx) {
        if (ctx.typeName() == null) {
            return visit(ctx.unaryExpression());
        } else {
            Type type = visit(ctx.typeName());
            CastExpression castExpression = visit(ctx.castExpression());
            return new CastExpressionImpl(type, castExpression);
        }
    }

    private UnaryExpression visit(CParser.UnaryExpressionContext ctx) {
        if (ctx.postfixExpression() != null) {
            return visit(ctx.postfixExpression());
        } else if (ctx.castExpression() != null) {
            UnaryExpressionUnaryOperatorImpl.UnaryOperator unaryOperator = visit(ctx.unaryOperator());
            CastExpression castExpression = visit(ctx.castExpression());
            return new UnaryExpressionUnaryOperatorImpl(unaryOperator, castExpression);
        } else {
            String firstToken = ctx.getChild(0).getText();
            UnaryExpression unaryExpression;
            switch (firstToken) {
                case "++":
                    unaryExpression = visit(ctx.unaryExpression());
                    return new UnaryExpressionIncrementImpl(UnaryExpressionIncrementImpl.IncrementOperator.PREFIX_INCREMENT, unaryExpression);
                case "--":
                    unaryExpression = visit(ctx.unaryExpression());
                    return new UnaryExpressionIncrementImpl(UnaryExpressionIncrementImpl.IncrementOperator.PREFIX_DECREMENT, unaryExpression);
                case "sizeof":
                    if (ctx.unaryExpression() != null) {
                        unaryExpression = visit(ctx.unaryExpression());
                        return new UnaryExpressionSizeofExpressionImpl(unaryExpression);
                    } else {
                        Type type = visit(ctx.typeName());
                        return new UnaryExpressionSizeofTypeImpl(type);
                    }
                default:
                    throw new IllegalArgumentException("Token not recognized " + firstToken);
            }
        }
    }

    private UnaryExpressionUnaryOperatorImpl.UnaryOperator visit(CParser.UnaryOperatorContext ctx) {
        return UnaryExpressionUnaryOperatorImpl.UnaryOperator.toUnaryOperator(ctx.getText());
    }

    private PostfixExpression visit(CParser.PostfixExpressionContext ctx) {
        if (ctx.primaryExpression() != null) {
            return visit(ctx.primaryExpression());
        } else {
            PostfixExpression postfixExpression = visit(ctx.postfixExpression());
            String token = ctx.getChild(1).getText();
            String identifier;
            switch (token) {
                case "[":
                    Expression expression = visit(ctx.expression());
                    return new PostfixExpressionArrayAccessImpl(postfixExpression, expression);
                case "(":
                    List<AssignmentExpression> assignmentExpressions = visit(ctx.argumentExpressionList());
                    return new PostfixExpressionInvocationImpl(postfixExpression, assignmentExpressions);
                case ".":
                    identifier = ctx.Identifier().getSymbol().getText();
                    return new PostfixExpressionStructAccessImpl(postfixExpression, PostfixExpressionStructAccessImpl.AccessOperator.DOT, identifier);
                case "->":
                    identifier = ctx.Identifier().getSymbol().getText();
                    return new PostfixExpressionStructAccessImpl(postfixExpression, PostfixExpressionStructAccessImpl.AccessOperator.ARROW, identifier);
                case "++":
                    return new PostfixExpressionIncrementImpl(postfixExpression, PostfixExpressionIncrementImpl.PostfixOperator.POSTFIX_INCREMENT);
                case "--":
                    return new PostfixExpressionIncrementImpl(postfixExpression, PostfixExpressionIncrementImpl.PostfixOperator.POSTFIX_DECREMENT);
                default:
                    throw new UnsupportedOperationException("What kind of postfix expression is this? " + ctx);
            }
        }
    }

    private List<AssignmentExpression> visit(CParser.ArgumentExpressionListContext ctx) {
        List<AssignmentExpression> assignmentExpressions = new ArrayList<>();
        List<CParser.AssignmentExpressionContext> assignmentExpressionContexts = ctx.assignmentExpression();
        return assignmentExpressionContexts.stream().map(asgn -> visit(asgn)).collect(Collectors.toList());
    }

    private PrimaryExpression visit(CParser.PrimaryExpressionContext ctx) {
        if (ctx.Identifier() != null) {
            return new PrimaryExpressionIdentifier(ctx.Identifier().getSymbol().getText());
        } else if (ctx.Constant() != null) {
            String token = ctx.Constant().getSymbol().getText();
            try {
                return new PrimaryExpressionConstant(Long.valueOf(token));
            } catch (NumberFormatException e) {

            }
            try {
                return new PrimaryExpressionConstant(Double.valueOf(token));
            } catch (NumberFormatException e) {

            }
            try {
                if (token.length() == 1) {
                    return new PrimaryExpressionConstant(Character.valueOf(token.charAt(0)));
                }
            } catch (NumberFormatException e) {

            }
        } else if (ctx.StringLiteral() != null && ctx.StringLiteral().size() > 0) {
            List<TerminalNode> terminalNodes = ctx.StringLiteral();
            for (TerminalNode terminalNode : terminalNodes) {
                return new PrimaryExpressionConstant(terminalNode.getSymbol().getText());
            }
        } else if (ctx.expression() != null) {
            return new PrimaryExpressionParentheses(visit(ctx.expression()));
        } else {
            throw new IllegalArgumentException("What kind of primary expression is this? " + ctx.getText());
        }
        throw new IllegalArgumentException("Fell through");
    }

    private Type visit(CParser.TypeNameContext ctx) {
        CParser.SpecifierQualifierListContext specifierQualifierListContext = ctx.specifierQualifierList();
        Type type = null;
        while (specifierQualifierListContext != null) {
            if (specifierQualifierListContext.typeSpecifier() != null) {
                CParser.TypeSpecifierContext typeSpecifierContext = specifierQualifierListContext.typeSpecifier();
                type = visit(typeSpecifierContext);
                System.out.println(type);
                break;
            }
            specifierQualifierListContext = specifierQualifierListContext.specifierQualifierList();
        }

        if (ctx.abstractDeclarator() != null) {
            int nestedPointers = visit(ctx.abstractDeclarator().pointer());
            return new PointerType(nestedPointers, (ActualType)type);
        } else {
            return type;
        }
    }

    private int visit(CParser.PointerContext ctx) {
        int nestedPointers = 0;
        while (ctx != null) {
            nestedPointers++;
            ctx = ctx.pointer();
        }
        return nestedPointers;
    }

    private Type getType(CParser.DeclarationSpecifiersContext declarationSpecifiersContext, CParser.DeclaratorContext declaratorContext) {
        Type type = visit(declarationSpecifiersContext);
        if (declaratorContext.pointer() != null) {
            int nestedPointers = visit(declaratorContext.pointer());
            return new PointerType(nestedPointers, (ActualType)type);
        } else {
            return type;
        }
    }

    private Type visit(CParser.TypeSpecifierContext ctx) {
        if (ctx.atomicTypeSpecifier() != null) {
            return visit(ctx.atomicTypeSpecifier().typeName());
        } else if (ctx.structOrUnionSpecifier() != null) {
            return new StructUnionType(ctx.structOrUnionSpecifier().Identifier().getSymbol().getText());
        } else if (ctx.enumSpecifier() != null) {
            throw new UnsupportedOperationException("Enum has been visited. Please implement");
        } else if (ctx.typedefName() != null) {
            return new TypedefType(ctx.typedefName().Identifier().getSymbol().getText());
        } else {
            return new PrimitiveType(ctx.getChild(0).getText());
        }
    }
}
