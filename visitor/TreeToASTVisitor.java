package visitor;

import ast.*;
import ast.Function;
import ast.type.*;
import jdk.nashorn.internal.ir.Block;
import org.antlr.v4.codegen.model.decl.Decl;
import org.antlr.v4.runtime.RuleContext;
import org.antlr.v4.runtime.tree.*;
import parser.CParser;

import java.util.*;
import java.util.function.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/24/16.
 */
public class TreeToASTVisitor {

    public Program visit(CParser.CompilationUnitContext ctx) {
        Collection<Function> functions = new ArrayList<>();
        CParser.TranslationUnitContext translationUnitContext = ctx.translationUnit();
        while (translationUnitContext != null) {
            CParser.ExternalDeclarationContext externalDeclarationContext = translationUnitContext.externalDeclaration();
            if (externalDeclarationContext.functionDefinition() != null) {
                Function function = visit(externalDeclarationContext.functionDefinition());
                functions.add(function);
            } else if (externalDeclarationContext.declaration() != null) {
                //visit(externalDeclarationContext.declaration());
            }
            translationUnitContext = translationUnitContext.translationUnit();
        }

        System.out.println(functions);
        return new Program(functions, Collections.emptyList());
    }

    private String visit(CParser.DeclaratorContext ctx) {
        CParser.DirectDeclaratorContext currentContext = ctx.directDeclarator();
        if (currentContext.declarator() != null) {
            return visit(currentContext.declarator());
        }
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
        List<CParser.DeclarationSpecifierContext> declarationSpecifier = ctx.declarationSpecifier().stream().filter(dec -> dec.typeSpecifier() != null).collect(Collectors.toList());
        if (declarationSpecifier.size() == 1) {
            CParser.DeclarationSpecifierContext type = declarationSpecifier.stream().findAny().get();
            RuleContext ruleNode = type.typeSpecifier().getRuleContext();
            for (int i = 0; i < ruleNode.getChildCount(); i++) {
                ParseTree innerTree = ruleNode.getChild(i);
                return new PrimitiveType(innerTree.getText());
            }
        } else {
            System.err.println(declarationSpecifier.stream().map(dec -> dec.getText()).collect(Collectors.toList()));
            System.err.println("More than one type?");
        }
        return null;
    }

    private ParameterList visit(CParser.ParameterTypeListContext ctx) {
        if (ctx != null) {
            return visit(ctx.parameterList());
        } else {
            return new ParameterList();
        }
    }

    private ParameterList visit(CParser.ParameterListContext ctx) {
        List<Parameter> parameters = new ArrayList<>();
        while (ctx != null) {
            Parameter parameter = visit(ctx.parameterDeclaration());
            parameters.add(parameter);
            ctx = ctx.parameterList();
        }
        return new ParameterList(parameters);
    }

    private Parameter visit(CParser.ParameterDeclarationContext ctx) {
        Type type = getType(ctx.declarationSpecifiers(), ctx.declarator());
        String identifier = visit(ctx.declarator());
        return new Parameter(type, identifier);
    }

    private CompoundStatement visit(CParser.CompoundStatementContext ctx) {
        List<BlockItem> blockItems = visit(ctx.blockItemList());
        return new CompoundStatement(blockItems);
    }

    private List<BlockItem> visit(CParser.BlockItemListContext ctx) {
        List<BlockItem> blockItems;
        if (ctx.blockItemList() != null) {
            blockItems = visit(ctx.blockItemList());
        } else {
            blockItems = new ArrayList<>();
        }
        blockItems.add(visit(ctx.blockItem()));
        return blockItems;
    }

    private BlockItem visit(CParser.BlockItemContext ctx) {
        if (ctx.declaration() != null) {
            return visit(ctx.declaration());
        } else {
            return visit(ctx.statement());
        }
    }

    private Declaration visit(CParser.DeclarationContext ctx) {
        Type type = visit(ctx.declarationSpecifiers());
        CParser.InitDeclaratorListContext initDeclaratorListContext = ctx.initDeclaratorList();
        List<Declaration.DeclaredVariable> declaredVariables = new ArrayList<>();
        while (initDeclaratorListContext != null) {
            Declaration.DeclaredVariable declaredVariable = visit(initDeclaratorListContext.initDeclarator(), type);
            declaredVariables.add(declaredVariable);
            initDeclaratorListContext = initDeclaratorListContext.initDeclaratorList();
        }
        return new Declaration(declaredVariables);
    }

    private Declaration.DeclaredVariable visit(CParser.InitDeclaratorContext ctx, Type type) {
        String identifier = visit(ctx.declarator());
        if (ctx.declarator().pointer() != null) {
            int nestedPointers = visit(ctx.declarator().pointer());
            type = new PointerType(nestedPointers, type);
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
        System.err.println("Statement returning null");
        return null;
    }

    private LabeledStatement visit(CParser.LabeledStatementContext ctx) {
        return null;
    }

    private ExpressionStatement visit(CParser.ExpressionStatementContext ctx) {
        if (ctx.expression() != null) {
            Expression expression = visit(ctx.expression());
            return new ExpressionStatement(expression);
        }
        System.err.println("ExpressionStatement returning null");
        return null;
    }

    private SelectionStatement visit(CParser.SelectionStatementContext ctx) {
        System.out.println(ctx.getRuleIndex());
        return null;
    }

    private IterationStatement visit(CParser.IterationStatementContext ctx) {
        System.out.println(ctx.getRuleIndex());
        return null;
    }

    private JumpStatement visit(CParser.JumpStatementContext ctx) {
        List<ParseTree> children = ctx.children;
        switch (children.get(0).getText()) {
            case "continue":
                return new JumpContinueStatement();
            case "break":
                return new JumpBreakStatement();
            case "return":
                Expression returnExpression = visit(ctx.expression());
                return new JumpReturnStatement(returnExpression);
            default:
                return null;
        }
    }

    private Expression visit(CParser.ExpressionContext ctx) {
        AssignmentExpression assignmentExpression = visit(ctx.assignmentExpression());
        if (ctx.expression() != null) {
            List<AssignmentExpression> assignmentExpressions = new ArrayList<>();
            assignmentExpressions.add(assignmentExpression);
            while (ctx.expression() != null) {
                ctx = ctx.expression();
                assignmentExpression = visit(ctx.assignmentExpression());
                assignmentExpressions.add(assignmentExpression);
            }
            return new CommaExpression(assignmentExpressions);
        } else {
            return assignmentExpression;
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
        LogicalAndExpression logicalAndExpression = visit(ctx.logicalAndExpression());
        if (ctx.logicalOrExpression() != null) {
            Collection<LogicalAndExpression> logicalAndExpressions = new ArrayList<>();
            logicalAndExpressions.add(logicalAndExpression);
            while (ctx.logicalOrExpression() != null) {
                ctx = ctx.logicalOrExpression();
                logicalAndExpression = visit(ctx.logicalAndExpression());
                logicalAndExpressions.add(logicalAndExpression);

            }
            return new LogicalOrExpressionImpl(logicalAndExpressions);
        } else {
            return logicalAndExpression;
        }
    }

    private LogicalAndExpression visit(CParser.LogicalAndExpressionContext ctx) {
        InclusiveBitwiseOrExpression inclusiveBitwiseOrExpression = visit(ctx.inclusiveOrExpression());
        if (ctx.logicalAndExpression() != null) {
            Collection<InclusiveBitwiseOrExpression> inclusiveBitwiseOrExpressions = new ArrayList<>();
            inclusiveBitwiseOrExpressions.add(inclusiveBitwiseOrExpression);
            while (ctx.logicalAndExpression() != null) {
                ctx = ctx.logicalAndExpression();
                inclusiveBitwiseOrExpression = visit(ctx.inclusiveOrExpression());
                inclusiveBitwiseOrExpressions.add(inclusiveBitwiseOrExpression);
            }
            return new LogicalAndExpressionImpl(inclusiveBitwiseOrExpressions);
        } else {
            return inclusiveBitwiseOrExpression;
        }
    }

    private InclusiveBitwiseOrExpression visit(CParser.InclusiveOrExpressionContext ctx) {
        ExclusiveBitwiseOrExpression exclusiveBitwiseOrExpression = visit(ctx.exclusiveOrExpression());
        if (ctx.inclusiveOrExpression() != null) {
            Collection<ExclusiveBitwiseOrExpression> exclusiveBitwiseOrExpressions = new ArrayList<>();
            exclusiveBitwiseOrExpressions.add(exclusiveBitwiseOrExpression);
            while (ctx.inclusiveOrExpression() != null) {
                ctx = ctx.inclusiveOrExpression();
                exclusiveBitwiseOrExpressions.add(visit(ctx.exclusiveOrExpression()));
            }
            return new InclusiveBitwiseOrExpressionImpl(exclusiveBitwiseOrExpressions);
        }
        return exclusiveBitwiseOrExpression;
    }

    private ExclusiveBitwiseOrExpression visit(CParser.ExclusiveOrExpressionContext ctx) {
        List<BitwiseAndExpression> bitwiseAndExpressions = new ArrayList<>();
        while (ctx != null) {
            bitwiseAndExpressions.add(visit(ctx.andExpression()));
            ctx = ctx.exclusiveOrExpression();
        }
        if (bitwiseAndExpressions.size() == 1) {
            return bitwiseAndExpressions.get(0);
        } else {
            return new ExclusiveBitwiseOrExpressionImpl(bitwiseAndExpressions);
        }
    }

    private BitwiseAndExpression visit(CParser.AndExpressionContext ctx) {
        List<EqualityExpression> equalityExpressions = new ArrayList<>();
        while (ctx != null) {
            equalityExpressions.add(visit(ctx.equalityExpression()));
            ctx = ctx.andExpression();
        }
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
            AdditiveExpression additiveExpression = visit(ctx.additiveExpression());
            AdditiveExpressionImpl.AdditiveOperator additiveOperator = AdditiveExpressionImpl.AdditiveOperator.toAdditiveOperator(ctx.getChild(1).getText());
            return new AdditiveExpressionImpl(additiveExpression, additiveOperator, multiplicativeExpression);
        } else {
            return multiplicativeExpression;
        }
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
                    return new UnaryExpressionImpl(UnaryExpressionImpl.IncrementOperator.PREFIX_INCREMENT, unaryExpression);
                case "--":
                    unaryExpression = visit(ctx.unaryExpression());
                    return new UnaryExpressionImpl(UnaryExpressionImpl.IncrementOperator.PREFIX_DECREMENT, unaryExpression);
                case "sizeof":
                    if (ctx.unaryExpression() != null) {
                        unaryExpression = visit(ctx.unaryExpression());
                        return new UnaryExpressionSizeofExpressionImpl(unaryExpression);
                    } else {
                        Type type = visit(ctx.typeName());
                        return new UnaryExpressionSizeofTypeImpl(type);
                    }
                default:
                    return null;
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
                    return new PostfixExpressionImpl(postfixExpression, PostfixExpressionImpl.PostfixOperator.POSTFIX_INCREMENT);
                case "--":
                    return new PostfixExpressionImpl(postfixExpression, PostfixExpressionImpl.PostfixOperator.POSTFIX_DECREMENT);
                default:
                    return null;
            }
        }
    }

    private List<AssignmentExpression> visit(CParser.ArgumentExpressionListContext ctx) {
        List<AssignmentExpression> assignmentExpressions = new ArrayList<>();
        while (ctx != null) {
            assignmentExpressions.add(visit(ctx.assignmentExpression()));
            ctx = ctx.argumentExpressionList();
        }
        return assignmentExpressions;
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
        } else if (ctx.StringLiteral() != null) {
            List<TerminalNode> terminalNodes = ctx.StringLiteral();
            for (TerminalNode terminalNode : terminalNodes) {
                return new PrimaryExpressionConstant(terminalNode.getSymbol().getText());
            }
        } else if (ctx.expression() != null) {
            return new PrimaryExpressionParentheses(visit(ctx.expression()));
        }
        return null;
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
            return new PointerType(nestedPointers, type);
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
            return new PointerType(nestedPointers, type);
        } else {
            return type;
        }
    }

    private Type visit(CParser.TypeSpecifierContext ctx) {
        if (ctx.structOrUnionSpecifier() != null) {
            return new StructUnionType(ctx.structOrUnionSpecifier().Identifier().getSymbol().getText());
        } else if (ctx.typedefName() != null) {
            return new TypedefType(ctx.typedefName().Identifier().getSymbol().getText());
        } else if (ctx.enumSpecifier() != null) {
            return new TypedefType(ctx.enumSpecifier().Identifier().getSymbol().getText());
        } else {
            String token = ctx.getChild(0).getText();
            return new PrimitiveType(token);
        }
    }
}
