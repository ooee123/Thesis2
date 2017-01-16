package visitor;

import ast.*;
import ast.declaration.TypedefDeclaration;
import ast.declaration.VariableDeclaration;
import ast.expression.*;
import ast.expression.impl.*;
import ast.statement.Statement;
import ast.statement.impl.*;
import ast.type.Type;
import lombok.Data;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.*;

import static ast.declaration.VariableDeclaration.DeclaredVariable;

@Data
public class ASTGeneralVisitor {
    private TypeScope globalScope;
    private Map<Statement, TypeScope> functionScopes;
    private Map<String, Type> functionTypes;
    private Program program;

    public ASTGeneralVisitor(Program p) {
        this.program = p;
        functionScopes = new IdentityHashMap<>();
        functionTypes = new HashMap<>();
        Map<String, Type> variableDeclarations = getVariableDeclarations(p.getVariableDeclarations().toArray(new VariableDeclaration[0]));
        Collection<TypedefDeclaration> typedefDeclarations = p.getTypedefDeclarations();
        globalScope = new TypeScope(variableDeclarations, null);
        Collection<Function> functions = p.getFunction();
        for (Function f : functions) {
            String functionId = f.getIdentifier();
            Type functionReturnType = f.getReturnType();
            functionTypes.put(functionId, functionReturnType);
            TypeScope functionScope = getDeclarations(f.getCompoundStatement(), globalScope);
            for (Parameter parameter : f.getParameterList().getParameters()) {
                functionScope.put(parameter.getFormalParameterName().getIdentifier(), parameter.getType());
            }
        }
    }

    private TypeScope getDeclarations(Statement stm, TypeScope parent) {
        if (stm instanceof SelectionStatementIf) {
            SelectionStatementIf selectionStatementIf = (SelectionStatementIf) stm;
            Statement thenStatement = selectionStatementIf.getThenStatement();
            TypeScope thenScope = getDeclarations(thenStatement, parent);
            functionScopes.put(thenStatement, thenScope);
            if (selectionStatementIf.getElseStatement() != null) {
                Statement elseStatement = selectionStatementIf.getElseStatement();
                TypeScope elseScope = getDeclarations(elseStatement, parent);
                functionScopes.put(elseStatement, elseScope);
            }
            return new TypeScope(parent);
        } else if (stm instanceof IterationStatementFor) {
            IterationStatementFor iterationStatementFor = (IterationStatementFor) stm;
            Statement body = iterationStatementFor.getStatement();
            TypeScope bodyScope = getDeclarations(body, parent);
            functionScopes.put(body, bodyScope);
            return bodyScope;
        } else if (stm instanceof IterationStatementDeclareFor) {
            IterationStatementDeclareFor iterationStatementDeclareFor = (IterationStatementDeclareFor) stm;
            Statement forBody = iterationStatementDeclareFor.getStatement();
            Map<String, Type> variableDeclarations = getVariableDeclarations(iterationStatementDeclareFor.getVariableDeclaration());
            TypeScope forBodyScope = getDeclarations(forBody, parent);
            forBodyScope.putAll(variableDeclarations);
            functionScopes.put(forBody, forBodyScope);
            return forBodyScope;
        } else if (stm instanceof IterationStatementWhile) {
            IterationStatementWhile iterationStatementWhile = (IterationStatementWhile) stm;
            Statement whileBody = iterationStatementWhile.getStatement();
            TypeScope whileScope = getDeclarations(whileBody, parent);
            functionScopes.put(whileBody, whileScope);
            return whileScope;
        } else if (stm instanceof IterationStatementDoWhile) {
            Statement whileBody = ((IterationStatementDoWhile) stm).getStatement();
            TypeScope whileScope = getDeclarations(whileBody, parent);
            functionScopes.put(whileBody, whileScope);
            return whileScope;
        } else if (stm instanceof CompoundStatement) {
            CompoundStatement compoundStatement = (CompoundStatement) stm;
            TypeScope newScope = new TypeScope(parent);
            Collection<BlockItem> blockItems = compoundStatement.getBlockItems();
            for (BlockItem blockItem : blockItems) {
                if (blockItem instanceof VariableDeclaration) {
                    newScope.putAll(getVariableDeclarations(((VariableDeclaration) blockItem)));
                } else if (blockItem instanceof Statement) {
                    getDeclarations(((Statement) blockItem), newScope);
                }
            }
            functionScopes.put(compoundStatement, newScope);
            return newScope;
        } else {
            return null;
        }
    }

    private Map<String, Type> getVariableDeclarations(VariableDeclaration... variableDeclarations) {
        Map<String, Type> declarationsMap = new HashMap<>();
        for (VariableDeclaration variableDeclaration : variableDeclarations) {
            for (DeclaredVariable declaredVariable : variableDeclaration.getDeclaredVariables()) {
                String id = declaredVariable.getIdentifier().getIdentifier();
                Type type = declaredVariable.getType();
                declarationsMap.put(id, type);
            }
        }
        return declarationsMap;
    }

    public void walk() {
        Collection<Function> functions = program.getFunction();
        for (Function function : functions) {
            walk(function.getCompoundStatement());
        }
    }

    public void walk(Statement statement) {
        if (statement instanceof ExpressionStatement) {
            Expression expression = ((ExpressionStatement) statement).getExpression();
            return;
        } else if (statement instanceof IterationStatementFor) {
            IterationStatementFor iterationStatementFor = (IterationStatementFor) statement;
            if (iterationStatementFor.getInitial() != null) {
                System.out.println(iterationStatementFor.getInitial().getGuaranteedChangedVariables());
            }
            if (iterationStatementFor.getCondition() != null) {
                System.out.println(iterationStatementFor.getCondition().getGuaranteedChangedVariables());
            }
            if (iterationStatementFor.getIteration() != null) {
                System.out.println(iterationStatementFor.getIteration().getGuaranteedChangedVariables());
            }

            walk(iterationStatementFor.getStatement());
        } else if (statement instanceof IterationStatementWhile) {
            IterationStatementWhile iterationStatementWhile = (IterationStatementWhile) statement;
            System.out.println(iterationStatementWhile.getCondition().getGuaranteedChangedVariables());
            walk(iterationStatementWhile.getStatement());
        } else if (statement instanceof SelectionStatementIf) {
            SelectionStatementIf selectionStatementIf = (SelectionStatementIf) statement;
            System.out.println(selectionStatementIf.getCondition().getGuaranteedChangedVariables());
            walk(selectionStatementIf.getThenStatement());
            if (selectionStatementIf.getElseStatement() != null) {
                walk(selectionStatementIf.getElseStatement());
            }
        } else if (statement instanceof IterationStatementDeclareFor) {
            IterationStatementDeclareFor iterationStatementDeclareFor = (IterationStatementDeclareFor) statement;
            walk(iterationStatementDeclareFor.getVariableDeclaration());
            if (iterationStatementDeclareFor.getCondition() != null) {
                System.out.println(iterationStatementDeclareFor.getCondition().getGuaranteedChangedVariables());
            }
            if (iterationStatementDeclareFor.getIteration() != null) {
                System.out.println(iterationStatementDeclareFor.getIteration().getGuaranteedChangedVariables());
            }
            walk(iterationStatementDeclareFor.getStatement());
        } else if (statement instanceof IterationStatementDoWhile) {
            IterationStatementDoWhile iterationStatementDoWhile = (IterationStatementDoWhile) statement;
            System.out.println(iterationStatementDoWhile.getCondition().getGuaranteedChangedVariables());
            walk(iterationStatementDoWhile.getStatement());
        } else if (statement instanceof CompoundStatement) {
            CompoundStatement compoundStatement = (CompoundStatement) statement;
            walk(compoundStatement);
        }
    }

    public void walk(CompoundStatement statement) {
        for (BlockItem blockItem : statement.getBlockItems()) {
            if (blockItem instanceof Statement) {
                walk((Statement) blockItem);
            } else if (blockItem instanceof VariableDeclaration) {
                walk(((VariableDeclaration) blockItem));
            }
        }
    }

    public void walk(Expression expression) {
        if (expression instanceof AssignmentExpressionImpl) {
            walk(((AssignmentExpressionImpl) expression).getUnaryExpression());
        } else if (expression instanceof CommaExpression) {
            for (AssignmentExpression assignmentExpression : ((CommaExpression) expression).getAssignmentExpressions()) {
                walk(assignmentExpression);
            }
        } else if (expression instanceof PrimaryExpressionParentheses) {
            walk(((PrimaryExpressionParentheses) expression).getExpression());
        }
    }

    public void walk(UnaryExpression expression) {
        if (expression instanceof PrimaryExpressionParentheses) {
        } else if (expression instanceof UnaryExpressionUnaryOperatorImpl) {
            UnaryExpressionUnaryOperatorImpl.UnaryOperator unaryOperator = ((UnaryExpressionUnaryOperatorImpl) expression).getUnaryOperator();
            if (unaryOperator.equals(UnaryExpressionUnaryOperatorImpl.UnaryOperator.DEREFERENCE)) {

            }
        } else if (expression instanceof UnaryExpressionIncrementImpl) {
            walk(((UnaryExpressionIncrementImpl) expression).getUnaryExpression());
        } else {
            System.out.println("LValue:");
            System.out.println(expression.getGuaranteedChangedVariables());
        }
    }

    public void walk(VariableDeclaration variableDeclaration) {
        for (DeclaredVariable declaredVariable : variableDeclaration.getDeclaredVariables()) {
            if (declaredVariable.getInitializer() != null) {
                AssignmentExpression initializer = declaredVariable.getInitializer();
                if (initializer instanceof AssignmentExpressionImpl) {
                    AssignmentExpressionImpl initializer1 = (AssignmentExpressionImpl) initializer;
                    System.out.println(initializer1.getGuaranteedChangedVariables());
                }
                System.out.println("[" + declaredVariable.getIdentifier() + "]");
            }
        }
    }

    public String diagnose() {

        StringWriter writer = new StringWriter();
        PrintWriter printer = new PrintWriter(writer);
        printer.println("Functions");
        Set<Map.Entry<String, Type>> entries = functionTypes.entrySet();
        for (Map.Entry<String, Type> entry : entries) {
            printer.println(entry.getKey() + " -> " + entry.getValue());
        }
        printer.println("Listing All Scopes...");
        printer.println("===global===");
        for (Map.Entry<String, Type> stringTypeEntry : globalScope.getTypes().entrySet()) {
            printer.println(stringTypeEntry.getKey() + " is a " + stringTypeEntry.getValue().toString());
        }
        Map<TypeScope, String> statementLabel = new IdentityHashMap<>();
        statementLabel.put(globalScope, "global");
        int count = 0;
        for (Map.Entry<Statement, TypeScope> entry : functionScopes.entrySet()) {
            String label = String.valueOf(Character.valueOf((char)('A' + count++)));
            statementLabel.put(entry.getValue(), label);

            printer.println("===" + label + "===");
            for (Map.Entry<String, Type> stringTypeEntry : entry.getValue().getTypes().entrySet()) {
                printer.println(stringTypeEntry.getKey() + " is a " + stringTypeEntry.getValue().toString());
            }
        }

        printer.println("Putting Dependencies");
        for (TypeScope typeScope : functionScopes.values()) {
            printer.println(statementLabel.get(typeScope) + " has parent " + statementLabel.get(typeScope.getParent()));
        }

        printer.flush();
        return writer.getBuffer().toString();
    }
}
