package visitor;

import ast.*;
import ast.type.PrimitiveType;
import ast.type.Type;
import org.antlr.v4.runtime.RuleContext;
import org.antlr.v4.runtime.tree.*;
import parser.CParser;

import java.util.*;
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
        CParser.DirectDeclaratorContext directDeclaratorContext = currentContext.directDeclarator();
        return directDeclaratorContext.Identifier().getText();
    }

    private Function visit(CParser.FunctionDefinitionContext ctx) {
        Type type = visit(ctx.declarationSpecifiers());
        String identifier = visit(ctx.declarator());
        ctx.declarator();
        ctx.declarator().directDeclarator();
        ctx.declarator().directDeclarator().parameterTypeList();
        ParameterList parameterList = visit(ctx.declarator().directDeclarator().parameterTypeList());

        return new Function(type, identifier, parameterList, null);
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
        Type type = visit(ctx.declarationSpecifiers());
        String identifier = visit(ctx.declarator());
        return new Parameter(type, identifier);
    }

    private CompoundStatement visit(CParser.CompoundStatementContext ctx) {
        CParser.BlockItemListContext blockItemListContext = ctx.blockItemList();
        while (blockItemListContext != null) {
            CParser.BlockItemContext blockItemContext = blockItemListContext.blockItem();
            if (blockItemContext.declaration() != null) {
                Declaration declaration = visit(blockItemContext.declaration());
            } else if (blockItemContext.statement() != null) {
                Statement statement = visit(blockItemContext.statement());
            }
        }
        return null;
    }

    private Declaration visit(CParser.DeclarationContext ctx) {
        Type type = visit(ctx.declarationSpecifiers());
        //ctx.initDeclaratorList();
        return new Declaration();
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
        System.out.println(ctx.getRuleIndex());
        return null;
    }

    private Expression visit(CParser.ExpressionContext ctx) {
        while (ctx != null) {
            CParser.AssignmentExpressionContext assignmentExpressionContext = ctx.assignmentExpression();
            visit(assignmentExpressionContext);
            ctx = ctx.expression();
        }
        return null;
    }

    private AssignmentExpression visit(CParser.AssignmentExpressionContext ctx) {
        if (ctx.conditionalExpression() != null) {
            visit(ctx.conditionalExpression());
        } else {
            ctx.unaryExpression();
            ctx.assignmentOperator();
            ctx.assignmentExpression();
        }
        return null;
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
        return null;
    }

    private LogicalAndExpression visit(CParser.LogicalAndExpressionContext ctx) {
        return null;
    }

    private InclusiveBitwiseOrExpression visit(CParser.InclusiveOrExpressionContext ctx) {
        return null;
    }

    private ExclusiveBitwiseOrExpression visit(CParser.ExclusiveOrExpressionContext ctx) {
        return null;
    }

    private BitwiseAndExpression visit(CParser.AndExpressionContext ctx) {
        return null;
    }

    private EqualityExpression visit(CParser.EqualityExpressionContext ctx) {
        return null;
    }

    private RelationalExpression visit(CParser.RelationalExpressionContext ctx) {
        return null;
    }

    private BitwiseShiftExpression visit(CParser.ShiftExpressionContext ctx) {
        return null;
    }

    private AdditiveExpression visit(CParser.AdditiveExpressionContext ctx) {
        return null;
    }

    private MultiplicativeExpression visit(CParser.MultiplicativeExpressionContext ctx) {
        return null;
    }

    private CastExpression visit(CParser.CastExpressionContext ctx) {
        return null;
    }

    private UnaryExpression visit(CParser.UnaryExpressionContext ctx) {
        return null;
    }

    private PostfixExpression visit(CParser.PostfixExpressionContext ctx) {
        return null;
    }

    private PrimaryExpression visit(CParser.PrimaryExpressionContext ctx) {
        return null;
    }
}
