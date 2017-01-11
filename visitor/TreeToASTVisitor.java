package visitor;

import ast.*;
import ast.declaration.EnumDefinition;
import ast.declaration.FunctionPrototypeDeclaration;
import ast.declaration.StructDefinition;
import ast.declaration.TypedefDeclaration;
import ast.expression.*;
import ast.expression.impl.*;
import ast.statement.*;
import ast.statement.impl.*;
import ast.type.*;
import com.google.common.collect.Lists;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.TerminalNode;
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

    private Map<String, StructUnionType> tagMapper = new HashMap<>();
    private Map<String, EnumType> enumTagMapper = new HashMap<>();
    private Map<String, TypedefType> typedefMapper = new HashMap<>();
    private Map<String, Function> functions = new LinkedHashMap<>();

    public Program visit(CParser.CompilationUnitContext ctx) {
        /* Init */
        typedefMapper.put("__builtin_va_list", new TypedefType(new PrimitiveType("__builtin_va_list"), "__builtin_va_list"));


        Collection<Declaration> variableDeclarations = new ArrayList<>();
        CParser.TranslationUnitContext translationUnitContext = ctx.translationUnit();
        List<CParser.ExternalDeclarationContext> externalDeclarationContexts = translationUnitContext.externalDeclaration();
        for (CParser.ExternalDeclarationContext externalDeclarationContext : externalDeclarationContexts) {
            if (externalDeclarationContext.functionDefinition() != null) {
                visit(externalDeclarationContext.functionDefinition());
            } else if (externalDeclarationContext.declaration() != null) {
                Declaration declaration = visit(externalDeclarationContext.declaration());
                variableDeclarations.add(declaration);
            }
        }
        return new Program(functions.values(), variableDeclarations);
    }

    private String visit(CParser.DeclaratorContext ctx) {
        CParser.DirectDeclaratorContext currentContext = ctx.directDeclarator();
        while (currentContext.Identifier() == null) {
            if (currentContext.directDeclarator() == null) {
                currentContext = currentContext.declarator().directDeclarator();
            } else {
                currentContext = currentContext.directDeclarator();
            }
        }
        return currentContext.Identifier().getSymbol().getText();
    }

    private String getIdentifier(CParser.DeclaratorWithoutDeclaratorContext ctx) {
        CParser.DirectDeclaratorWithoutDeclaratorContext currentContext = ctx.directDeclaratorWithoutDeclarator();
        String name = null;

        if (currentContext.Identifier() == null) {
            CParser.DirectDeclaratorContext directDeclaratorContext = currentContext.directDeclarator();
            while (directDeclaratorContext.Identifier() == null) {
                if (directDeclaratorContext.directDeclarator() != null) {
                    directDeclaratorContext = directDeclaratorContext.directDeclarator();
                } else {
                    return visit(directDeclaratorContext.declarator());
                }
            }
            if (name != null) {
                throw new IllegalArgumentException("Name already assigned");
            }
            name = directDeclaratorContext.Identifier().getSymbol().getText();
        } else {
            if (name != null) {
                throw new IllegalArgumentException("Name already assigned");
            }
            name = currentContext.Identifier().getSymbol().getText();
        }
        if (name == null) {
            throw new IllegalArgumentException("Name not found");
        }
        return name;
    }

    private Function visit(CParser.FunctionDefinitionContext ctx) {
        Type type = getType(ctx.declarationSpecifiers(), ctx.declarator().pointer());
        String identifier = visit(ctx.declarator());
        ParameterList parameterList = visit(ctx.declarator().directDeclarator().parameterTypeList());
        CompoundStatement compoundStatement = visit(ctx.compoundStatement());
        Function function = new Function(type, identifier, parameterList, compoundStatement);
        functions.put(identifier, function);
        return function;
    }

    private boolean hasArray(CParser.DirectDeclaratorContext ctx) {
        if (ctx.directDeclarator() != null) {
            if (ctx.getChild(1).getText().equals("[")) {
                return true;
            }
            return hasArray(ctx.directDeclarator());
        }
        return false;
    }

    private boolean hasArray(CParser.DirectDeclaratorWithoutDeclaratorContext ctx) {
        if (ctx.directDeclarator() != null) {
            if (ctx.getChild(1).getText().equals("[")) {
                return true;
            }
            return hasArray(ctx.directDeclarator());
        }
        return false;
    }

    private AssignmentExpression visitGetArray(CParser.DirectDeclaratorContext ctx) {
        if (hasArray(ctx)) {
            if (ctx.assignmentExpression() != null) {
                return visit(ctx.assignmentExpression());
            } else {
                return null;
            }
        }
        throw new IllegalArgumentException("Doesn't have an array. Please run hasArray first");
    }

    private AssignmentExpression visitGetArray(CParser.DirectDeclaratorWithoutDeclaratorContext ctx) {
        if (hasArray(ctx)) {
            if (ctx.assignmentExpression() != null) {
                return visit(ctx.assignmentExpression());
            } else {
                return null;
            }
        }
        throw new IllegalArgumentException("Doesn't have an array. Please run hasArray first");
    }

    private Type visit(CParser.DeclarationSpecifiersContext ctx) {
        List<CParser.TypeSpecifierContext> typeSpecifierContexts = new ArrayList<>();
        for (CParser.DeclarationSpecifierContext declarationSpecifierContext : ctx.declarationSpecifier()) {
            if (declarationSpecifierContext.typeSpecifier() != null) {
                typeSpecifierContexts.add(declarationSpecifierContext.typeSpecifier());
            }
        }
        List<PrimitiveType> primitiveTypes = new ArrayList<>();
        for (CParser.TypeSpecifierContext typeSpecifierContext : typeSpecifierContexts) {
            if (typeSpecifierContext.typedefName() != null || typeSpecifierContext.structOrUnionSpecifier() != null || typeSpecifierContext.enumSpecifier() != null) {
                if (typeSpecifierContexts.size() > 1) {
                    throw new IllegalArgumentException("Typedef name with extra stuff " + ctx.getText());
                } else {
                    if (typeSpecifierContext.typedefName() != null) {
                        if (!typedefMapper.containsKey(typeSpecifierContext.typedefName().getText())) {
                            System.err.println("Not in");
                        }
                        return typedefMapper.get(typeSpecifierContext.typedefName().getText());
                    } else if (typeSpecifierContext.structOrUnionSpecifier() != null) {
                        if (!tagMapper.containsKey(typeSpecifierContext.structOrUnionSpecifier().Identifier().getText())) {
                            System.err.println("Not in");
                        }
                        return tagMapper.get(typeSpecifierContext.structOrUnionSpecifier().Identifier().getText());
                    } else {
                        throw new IllegalArgumentException("Enum encountered");
                    }
                }
            } else {
                PrimitiveType primitiveType = new PrimitiveType(typeSpecifierContext.getText());
                primitiveTypes.add(primitiveType);
            }
        }
        if (primitiveTypes.size() > 1) {
            return new MultiplePrimitiveType(primitiveTypes);
        }
        return primitiveTypes.get(0);
    }

    private ParameterList visit(CParser.ParameterTypeListContext ctx) {
        if (ctx != null) {
            return visit(ctx.parameterList());
        } else {
            return new ParameterList();
        }
    }

    private AbstractParameterList abstractVisit(CParser.ParameterListContext ctx) {
        List<AbstractParameter> abstractParameters = new ArrayList<>();
        List<CParser.ParameterDeclarationContext> parameterDeclarationContexts = ctx.parameterDeclaration();
        for (CParser.ParameterDeclarationContext parameterDeclarationContext : parameterDeclarationContexts) {
            AbstractParameter visit = visit(parameterDeclarationContext);
            abstractParameters.add(visit);
        }
        return new AbstractParameterList(abstractParameters);
    }

    private ParameterList visit(CParser.ParameterListContext ctx) {
        List<Parameter> abstractParameters = new ArrayList<>();
        List<CParser.ParameterDeclarationContext> parameterDeclarationContexts = ctx.parameterDeclaration();
        for (CParser.ParameterDeclarationContext parameterDeclarationContext : parameterDeclarationContexts) {
            Parameter visit = parameterVisit(parameterDeclarationContext);
            abstractParameters.add(visit);
        }
        return new ParameterList(abstractParameters);
    }

    private AbstractParameter visit(CParser.ParameterDeclarationContext ctx) {
        if (ctx.declarator() != null) {
            return parameterVisit(ctx);
        } else {
            if (ctx.abstractDeclarator() != null) {
                Type type = getType(ctx.declarationSpecifiers(), ctx.abstractDeclarator().pointer());
                return new AbstractParameter(type);
            } else {
                return new AbstractParameter(getType(ctx.declarationSpecifiers(), null));
            }
        }
    }

    private Parameter parameterVisit(CParser.ParameterDeclarationContext ctx) {
        if (ctx.declarator() != null) {
            Type type = getType(ctx.declarationSpecifiers(), ctx.declarator().pointer());
            PrimaryExpressionIdentifier identifier = new PrimaryExpressionIdentifier(visit(ctx.declarator()), type);
            return new Parameter(type, identifier);
        } else {
            throw new IllegalArgumentException("Not a strongly defiend parameter");
        }
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
        List<BlockItem> blockItems = new ArrayList<>();
        if (ctx.blockItem() != null) {
            List<CParser.BlockItemContext> blockItemContexts = ctx.blockItem();
            for (CParser.BlockItemContext blockItemContext : blockItemContexts) {
                BlockItem visit = visit(blockItemContext);
                if (visit instanceof FunctionPrototypeDeclaration) {
                    Type returnType = ((FunctionPrototypeDeclaration) visit).getReturnType();
                    AbstractParameterList parameterList = ((FunctionPrototypeDeclaration) visit).getParameterList();
                    FunctionPointer functionPointer = new FunctionPointer(returnType, parameterList);
                    VariableDeclaration.DeclaredVariable declaredVariable = new VariableDeclaration.DeclaredVariable(functionPointer, new PrimaryExpressionIdentifier(((FunctionPrototypeDeclaration) visit).getIdentifier(), functionPointer));
                    visit = new VariableDeclaration(Lists.newArrayList(declaredVariable));
                }
                blockItems.add(visit);
            }
            return blockItems;
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

    private boolean isTypedef(CParser.DeclarationSpecifiersContext ctx) {
        for (CParser.DeclarationSpecifierContext declarationSpecifierContext : ctx.declarationSpecifier()) {
            if (declarationSpecifierContext.storageClassSpecifier() != null) {
                if (declarationSpecifierContext.storageClassSpecifier().getText().equals("typedef")) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean isFunctionPrototype(CParser.DeclarationContext ctx) {
        for (CParser.InitDeclaratorContext initDeclaratorContext : ctx.initDeclaratorList().initDeclarator()) {
            if (initDeclaratorContext.declaratorWithoutDeclarator() != null) {
                if (initDeclaratorContext.declaratorWithoutDeclarator().directDeclaratorWithoutDeclarator() != null) {
                    if (initDeclaratorContext.declaratorWithoutDeclarator().directDeclaratorWithoutDeclarator().parameterTypeList() != null) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private FunctionPrototypeDeclaration processFunctionPrototype(CParser.DeclarationContext ctx) {
        Type type = visit(ctx.declarationSpecifiers());
        String functionName = getIdentifier(ctx.initDeclaratorList().initDeclarator().get(0).declaratorWithoutDeclarator());
        AbstractParameterList parameterList = abstractVisit(ctx.initDeclaratorList().initDeclarator().get(0).declaratorWithoutDeclarator().directDeclaratorWithoutDeclarator().parameterTypeList().parameterList());
        return new FunctionPrototypeDeclaration(type, functionName, parameterList);
    }

    private Declaration visit(CParser.DeclarationContext ctx) {
        if (ctx.structOrUnionSpecifier() != null) {
            StructUnionType visit = visit(ctx.structOrUnionSpecifier());
            return new StructDefinition(visit);
        } else if (ctx.enumSpecifier() != null) {
            EnumType visit = visit(ctx.enumSpecifier());
            return new EnumDefinition(visit);
        }
        boolean isTypeDef = isTypedef(ctx.declarationSpecifiers());
        if (isTypeDef) {
            return processTypedef(ctx);
        } else {
            /* Not a typedef: either a struct declaration, enum declaration, or variable declaration or function prototype
             */
            boolean isFunctionPrototype = isFunctionPrototype(ctx);
            if (isFunctionPrototype) {
                FunctionPrototypeDeclaration functionPrototypeDeclaration = processFunctionPrototype(ctx);
                return functionPrototypeDeclaration;
            }
            Type type = visit(ctx.declarationSpecifiers());
            if (ctx.initDeclaratorList() != null) {
                CParser.InitDeclaratorListContext initDeclaratorListContext = ctx.initDeclaratorList();
                List<CParser.InitDeclaratorContext> initDeclaratorContexts = initDeclaratorListContext.initDeclarator();
                List<VariableDeclaration.DeclaredVariable> declaredVariables = initDeclaratorContexts.stream().map(init -> visit(init, type)).collect(Collectors.toList());
                return new VariableDeclaration(declaredVariables);
            } else {
                if (!(type instanceof StructUnionType)) {
                    throw new IllegalArgumentException();
                }
                return new StructDefinition(((StructUnionType) type));
            }
        }
    }

    private List<CParser.TypeSpecifierContext> filterDeclarationContext(CParser.DeclarationContext ctx) {
        List<CParser.TypeSpecifierContext> typeSpecifierContexts = new ArrayList<>();
        for (CParser.DeclarationSpecifierContext declarationSpecifierContext : ctx.declarationSpecifiers().declarationSpecifier()) {
            if (declarationSpecifierContext.typeSpecifier() != null) {
                typeSpecifierContexts.add(declarationSpecifierContext.typeSpecifier());
            }
        }
        return typeSpecifierContexts;
    }

    private TypedefDeclaration processTypedef(CParser.DeclarationContext ctx) {
        String typedefName = getTypedefName(ctx);
        // Get all the inner parts
        Type originalType;
        List<CParser.TypeSpecifierContext> typeSpecifierContexts = filterDeclarationContext(ctx);
        if (ctx.initDeclaratorList().initDeclarator().get(0).declaratorWithoutDeclarator().pointer() == null) {
            originalType = visit(typeSpecifierContexts);
        } else {
            visit(typeSpecifierContexts);
            originalType = getType(ctx.declarationSpecifiers(), ctx.initDeclaratorList().initDeclarator().get(0).declaratorWithoutDeclarator().pointer());
        }
        TypedefType typedefType = new TypedefType(originalType, typedefName);
        typedefMapper.putIfAbsent(typedefName, typedefType);
        return new TypedefDeclaration(typedefType);
    }


    private String getTypedefName(CParser.DeclarationContext ctx) {
        for (CParser.InitDeclaratorContext initDeclaratorContext : ctx.initDeclaratorList().initDeclarator()) {
            CParser.DeclaratorWithoutDeclaratorContext declaratorWithoutDeclaratorContext = initDeclaratorContext.declaratorWithoutDeclarator();
            return getIdentifier(declaratorWithoutDeclaratorContext);
        }
        throw new IllegalArgumentException("No typedef identifier found?");
    }

    private VariableDeclaration.DeclaredVariable visit(CParser.InitDeclaratorContext ctx, Type type) {
        PrimaryExpressionIdentifier identifier = new PrimaryExpressionIdentifier(getIdentifier(ctx.declaratorWithoutDeclarator()), type);
        if (ctx.declaratorWithoutDeclarator().pointer() != null) {
            int nestedPointers = visit(ctx.declaratorWithoutDeclarator().pointer());
            type = new PointerType(nestedPointers, (ActualType)type);
        }
        AssignmentExpression arrayInitialSize = null;
        boolean hasArray = hasArray(ctx.declaratorWithoutDeclarator().directDeclaratorWithoutDeclarator());
        if (hasArray) {
            arrayInitialSize = visitGetArray(ctx.declaratorWithoutDeclarator().directDeclaratorWithoutDeclarator());
        }
        AssignmentExpression assignmentExpression = null;
        if (ctx.initializer() != null) {
            assignmentExpression = visit(ctx.initializer());
        }
        return new VariableDeclaration.DeclaredVariable(type, identifier, assignmentExpression, hasArray, arrayInitialSize);
    }

    private AssignmentExpression visit(CParser.InitializerContext ctx) {
        if (ctx.assignmentExpression() != null) {
            return visit(ctx.assignmentExpression());
        } else {
            List<AssignmentExpression> assignmentExpressions = new ArrayList<>();
            CParser.InitializerListContext initializerListContext = ctx.initializerList();
            for (CParser.InitializerContext initializerContext : initializerListContext.initializer()) {
                assignmentExpressions.add(visit(initializerContext));
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
        } else {
            if (ctx.logicalOrExpression() != null) {

            }
        }
        throw new UnsupportedOperationException("What kind of statement is this?\n" + ctx.getText());
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
        } else {
            return new ExpressionStatement(null);
        }
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
            return visit(ctx.switchStatement());
        }
    }

    private SelectionStatementSwitch visit(CParser.SwitchStatementContext ctx) {
        Expression expression = visit(ctx.expression());
        CompoundStatement statement = visit(ctx.compoundStatement());
        return new SelectionStatementSwitch(expression, statement);
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
        VariableDeclaration variableDeclaration = (VariableDeclaration)visit(ctx.declaration());
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
        return new IterationStatementDeclareFor(variableDeclaration, expression, iteration, statement);
    }

    private JumpStatement visit(CParser.JumpStatementContext ctx) {
        List<ParseTree> children = ctx.children;
        switch (children.get(0).getText()) {
            case "continue":
                return new JumpContinueStatement();
            case "break":
                return new JumpBreakStatement();
            case "return":
                if (ctx.expression() != null) {
                    return new JumpReturnStatement(visit(ctx.expression()));
                } else {
                    return new JumpReturnStatement(null);
                }

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
        List<MultiplicativeExpression> addingTerms = new ArrayList<>();
        List<MultiplicativeExpression> subtractingTerms = new ArrayList<>();

        while (ctx.additiveExpression() != null) {
            MultiplicativeExpression multiplicativeExpression = visit(ctx.multiplicativeExpression());
            AdditiveExpression.AdditiveOperator additiveOperator = AdditiveExpression.AdditiveOperator.toAdditiveOperator(ctx.getChild(1).getText());
            switch (additiveOperator) {
                case PLUS:
                    addingTerms.add(0, multiplicativeExpression);
                    break;
                case MINUS:
                    subtractingTerms.add(0, multiplicativeExpression);
                    break;
                default:
                    throw new IllegalArgumentException("NOT PLUS OR MINUS?" + ctx.getChild(1).getText());
            }
            ctx = ctx.additiveExpression();
        }
        addingTerms.add(0, visit(ctx.multiplicativeExpression()));
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
                        if (unaryExpression instanceof PrimaryExpressionParentheses) {
                            PrimaryExpressionParentheses unaryExpression1 = (PrimaryExpressionParentheses) unaryExpression;
                            if (unaryExpression1.getExpression() instanceof PrimaryExpressionIdentifier) {
                                String identifier = ((PrimaryExpressionIdentifier) unaryExpression1.getExpression()).getIdentifier();
                                if (typedefMapper.containsKey(identifier)) {
                                    return new UnaryExpressionSizeofTypeImpl(typedefMapper.get(identifier));
                                }
                            }
                        }
                        return new UnaryExpressionSizeofExpressionImpl(unaryExpression);
                    } else {
                        Type type = visit(ctx.typeName());
                        return new UnaryExpressionSizeofTypeImpl(type);
                        //throw new IllegalArgumentException("this shouldn't ever fall through because of parsing");
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
                    return new PostfixExpressionInvocationImpl(postfixExpression, assignmentExpressions, functions.get(postfixExpression.toCode()));
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
        if (ctx != null) {
            List<CParser.AssignmentExpressionContext> assignmentExpressionContexts = ctx.assignmentExpression();
            return assignmentExpressionContexts.stream().map(asgn -> visit(asgn)).collect(Collectors.toList());
        } else {
            return new ArrayList<>();
        }
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
                return new PrimaryExpressionConstant(Long.decode(token));
            } catch (NumberFormatException e) {

            }
            try {
                return new PrimaryExpressionConstant(Double.valueOf(token));
            } catch (NumberFormatException e) {

            }
            try {
                if (token.length() == 3) {
                    return new PrimaryExpressionConstant(Character.valueOf(token.charAt(1)));
                }
            } catch (NumberFormatException e) {

            }
            System.out.println("Replacing token: " + token);
            token = token.replaceAll("u", "");
            token = token.replaceAll("U", "");
            token = token.replaceAll("l", "");
            token = token.replaceAll("L", "");
            System.out.println("Got: " + token);
            try {
                return new PrimaryExpressionConstant(Long.valueOf(token));
            } catch (NumberFormatException e) {

            }
            try {
                return new PrimaryExpressionConstant(Long.decode(token));
            } catch (NumberFormatException e) {

            }
            /*
            if (ctx.constant().integerConstant() != null) {
                String token;
                if (ctx.constant().integerConstant().DecimalConstant() != null) {
                    token = ctx.constant().integerConstant().DecimalConstant().getSymbol().getText();
                } else if (ctx.constant().integerConstant().HexadecimalConstant() != null) {
                    token = ctx.constant().integerConstant().HexadecimalConstant().getSymbol().getText();
                } else if (ctx.constant().integerConstant().OctalConstant() != null) {
                    token = ctx.constant().integerConstant().OctalConstant().getSymbol().getText();
                } else {
                    token = ctx.constant().integerConstant().BinaryConstant().getSymbol().getText();
                }
                try {
                    return new PrimaryExpressionConstant(Long.valueOf(token));
                } catch (NumberFormatException e) {

                }
                try {
                    return new PrimaryExpressionConstant(Long.decode(token));
                } catch (NumberFormatException e) {

                }
            } else if (ctx.constant().floatingConstant() != null) {
                String token = ctx.constant().floatingConstant().getText();
                try {
                    return new PrimaryExpressionConstant(Double.valueOf(token));
                } catch (NumberFormatException e) {

                }
            } else if (ctx.constant().characterConstant() != null) {
                String token = ctx.constant().characterConstant().getText();
                try {
                    if (token.length() == 1) {
                        return new PrimaryExpressionConstant(Character.valueOf(token.charAt(0)));
                    }
                } catch (NumberFormatException e) {

                }
            }
            */
            throw new IllegalArgumentException("Didn't work with token: " + ctx.Constant().getText());
        } else if (ctx.StringLiteral() != null && ctx.StringLiteral().size() > 0) {
            List<TerminalNode> terminalNodes = ctx.StringLiteral();
            for (TerminalNode terminalNode : terminalNodes) {
                String substring = terminalNode.getSymbol().getText().substring(1, terminalNode.getSymbol().getText().length() - 1);
                return new PrimaryExpressionConstant(substring);
            }
            throw new IllegalArgumentException("Uh oh");
        } else if (ctx.expression() != null) {
            return new PrimaryExpressionParentheses(visit(ctx.expression()));
        } else {
            throw new IllegalArgumentException("What kind of primary expression is this? " + ctx.getText());
        }
    }

    private Type visit(CParser.TypeNameContext ctx) {
        CParser.SpecifierQualifierListContext specifierQualifierListContext = ctx.specifierQualifierList();
        Type type = null;
        for (CParser.TypeSpecifierContext typeSpecifierContext : specifierQualifierListContext.typeSpecifier()) {
            type = visit(typeSpecifierContext);
            if (ctx.abstractDeclarator() != null) {
                int nestedPointers = visit(ctx.abstractDeclarator().pointer());
                return new PointerType(nestedPointers, (ActualType)type);
            } else {
                return type;
            }
        }
        throw new IllegalArgumentException("Type not found");
    }

    private int visit(CParser.PointerContext ctx) {
        int nestedPointers = 0;
        while (ctx != null) {
            nestedPointers++;
            ctx = ctx.pointer();
        }
        return nestedPointers;
    }

    private Type getType(CParser.DeclarationSpecifiersContext declarationSpecifiersContext, CParser.PointerContext pointerContext) {
        Type type = visit(declarationSpecifiersContext);
        if (pointerContext == null) {
            return type;
        } else {
            int nestedPointers = visit(pointerContext);
            return new PointerType(nestedPointers, (ActualType) type);
        }
    }

    private Type visit(CParser.TypeSpecifierContext ctx) {
            if (ctx.atomicTypeSpecifier() != null) {
                return visit(ctx.atomicTypeSpecifier().typeName());
            } else if (ctx.structOrUnionSpecifier() != null) {
                return visit(ctx.structOrUnionSpecifier());
            } else if (ctx.enumSpecifier() != null) {
                return visit(ctx.enumSpecifier());
            } else if (ctx.typedefName() != null) {
                if (!typedefMapper.containsKey(ctx.typedefName().getText())) {
                    //typedefMapper.put(ctx.typedefName().getText(), new TypedefType());
                    throw new IllegalArgumentException("This type hasn't been recorded yet...");
                }
                return typedefMapper.get(ctx.typedefName().getText());
            } else {
                return new PrimitiveType(ctx.getChild(0).getText());
            }
    }

    // Gets the type from a list of type specifiers
    // Ex. unsigned long int
    private Type visit(List<CParser.TypeSpecifierContext> ctx) {
        if (ctx.size() == 1) {
            return visit(ctx.get(0));
        } else {
            List<PrimitiveType> primitiveTypes = new ArrayList<>();
            for (CParser.TypeSpecifierContext typeSpecifierContext : ctx) {
                Type visit = visit(typeSpecifierContext);
                primitiveTypes.add((PrimitiveType)visit);
            }
            return new MultiplePrimitiveType(primitiveTypes);
        }
    }

    private EnumType visit(CParser.EnumSpecifierContext ctx) {
        String identifier = null;
        if (ctx.Identifier() != null) {
            identifier = ctx.Identifier().getSymbol().getText();
        }
        if (typedefMapper.containsKey(identifier)) {
            return enumTagMapper.get(identifier);
        } else {
            EnumType enumType = new EnumType(identifier);
            for (CParser.EnumeratorContext enumeratorContext : ctx.enumeratorList().enumerator()) {
                String enumString = enumeratorContext.enumerationConstant().Identifier().getSymbol().getText();
                ConditionalExpression enumValue = null;
                if (enumeratorContext.constantExpression() != null) {
                    enumValue = visit(enumeratorContext.constantExpression().conditionalExpression());
                }
                enumType.addEnumValue(enumString, enumValue);
            }
            enumTagMapper.put(identifier, enumType);
            return enumType;
        }
    }

    private VariableDeclaration.DeclaredVariable grabSpecialStructField(CParser.StructDeclarationContext ctx) {

        List<CParser.TypeSpecifierContext> realTypeTypeSpecifierContexts = ctx.specifierQualifierList().typeSpecifier().subList(0, ctx.specifierQualifierList().typeSpecifier().size() - 1);
        Type visit = visit(realTypeTypeSpecifierContexts);
        CParser.TypeSpecifierContext identifierTypeSpecifierContext = ctx.specifierQualifierList().typeSpecifier().get(ctx.specifierQualifierList().typeSpecifier().size() - 1);
        String identifier = identifierTypeSpecifierContext.typedefName().Identifier().getSymbol().getText();
        return new VariableDeclaration.DeclaredVariable(visit, new PrimaryExpressionIdentifier(identifier));
    }

    private StructUnionType visit(CParser.StructOrUnionSpecifierContext ctx) {
        StructUnionType.StructUnion structUnion = StructUnionType.StructUnion.toStructUnion(ctx.structOrUnion().getText());
        List<VariableDeclaration> structVariableDeclarations = new ArrayList<>();

        String tag = null;
        if (ctx.Identifier() != null) {
            tag = ctx.Identifier().getSymbol().getText();
        }
        if (ctx.structDeclarationList() != null) {
            StructUnionType structUnionType = new StructUnionType(tag, structUnion, structVariableDeclarations);
            tagMapper.put(tag, structUnionType);
            for (CParser.StructDeclarationContext structDeclarationContext : ctx.structDeclarationList().structDeclaration()) {
                List<VariableDeclaration.DeclaredVariable> structMembers = new ArrayList<>();
                // Invoke special parsing rule if: structDeclarator.size() == 1 AND it has ":"
                if (structDeclarationContext.structDeclaratorList().structDeclarator().get(0).constantExpression() != null) {
                    CParser.TypedefNameContext lastIdentifier = structDeclarationContext.specifierQualifierList().typeSpecifier().get(structDeclarationContext.specifierQualifierList().typeSpecifier().size() - 1).typedefName();
                    if (lastIdentifier == null) {
                        // If it's a dummy space, an unnamed bit flag to fill up space
                        continue;
                    } else {
                        // So this is now a bit field
                        VariableDeclaration.DeclaredVariable declaredVariable1 = grabSpecialStructField(structDeclarationContext);
                        structMembers.add(declaredVariable1);
                    }
                } else {
                    Type visit = visit(structDeclarationContext.specifierQualifierList());
                    for (CParser.StructDeclaratorContext structDeclaratorContext : structDeclarationContext.structDeclaratorList().structDeclarator()) {
                        CParser.DeclaratorContext declarator = structDeclaratorContext.declarator();
                        String id = visit(structDeclaratorContext.declarator());
                        if (declarator.pointer() != null) {
                            int nestedPointers = visit(declarator.pointer());
                            visit = new PointerType(nestedPointers, (ActualType) visit);
                        }
                        VariableDeclaration.DeclaredVariable declaredVariable = new VariableDeclaration.DeclaredVariable(visit, new PrimaryExpressionIdentifier(id));
                        structMembers.add(declaredVariable);
                    }
                }
                VariableDeclaration variableDeclaration = new VariableDeclaration(structMembers);
                structVariableDeclarations.add(variableDeclaration);
            }
            return structUnionType;
        } else {
            if (!tagMapper.containsKey(tag)) {
                tagMapper.putIfAbsent(tag, new StructUnionType(tag, StructUnionType.StructUnion.STRUCT, null));
            }
            StructUnionType structUnionType = tagMapper.get(tag);
            return structUnionType;
        }
    }

    // Gets the type! and the type only!
    private Type visit(CParser.SpecifierQualifierListContext ctx) {
        Type visit = visit(ctx.typeSpecifier());
        if (visit == null) {
            throw new IllegalArgumentException();
        }
        return visit;
    }
}
