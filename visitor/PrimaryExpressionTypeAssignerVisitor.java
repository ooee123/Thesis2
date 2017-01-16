package visitor;

import ast.*;
import ast.declaration.TypedefDeclaration;
import ast.declaration.VariableDeclaration;
import ast.expression.Expression;
import ast.expression.impl.PrimaryExpressionIdentifier;
import ast.statement.Statement;
import ast.statement.impl.*;
import ast.type.Type;
import lombok.Data;

import java.util.*;

import static ast.declaration.VariableDeclaration.DeclaredVariable;

@Data
public class PrimaryExpressionTypeAssignerVisitor {
    private TypeScope globalScope;
    private Map<Statement, TypeScope> functionScopes;
    private Map<String, Type> functionTypes;
    private Program program;

    public PrimaryExpressionTypeAssignerVisitor(Program p) {
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

    private Visitor<Expression> generateTyper(TypeScope scope) {
        Visitor<Expression> primaryExpressionTyper = new Visitor<Expression>() {
            @Override
            public void visit(Expression expression) {
                if (expression instanceof PrimaryExpressionIdentifier) {
                    ((PrimaryExpressionIdentifier) expression).setType(scope.get(((PrimaryExpressionIdentifier) expression).getIdentifier()));
                } else {
                    expression.visitNestedExpressions(this);
                }
            }
        };
        return primaryExpressionTyper;
    }

    private TypeScope getDeclarations(Statement stm, TypeScope parent) {
        Visitor<Expression> currentScopeVisitor = generateTyper(parent);
        if (stm instanceof SelectionStatementIf) {
            SelectionStatementIf selectionStatementIf = (SelectionStatementIf) stm;
            Expression condition = selectionStatementIf.getCondition();
            condition.visitNestedExpressions(currentScopeVisitor);
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
            iterationStatementFor.getInitial().visitNestedExpressions(currentScopeVisitor);
            iterationStatementFor.getCondition().visitNestedExpressions(currentScopeVisitor);
            iterationStatementFor.getIteration().visitNestedExpressions(currentScopeVisitor);
            Statement body = iterationStatementFor.getStatement();
            TypeScope bodyScope = getDeclarations(body, parent);
            functionScopes.put(body, bodyScope);
            return bodyScope;
        } else if (stm instanceof IterationStatementDeclareFor) {
            IterationStatementDeclareFor iterationStatementDeclareFor = (IterationStatementDeclareFor) stm;
            Statement forBody = iterationStatementDeclareFor.getStatement();
            Map<String, Type> variableDeclarations = getVariableDeclarations(iterationStatementDeclareFor.getVariableDeclaration());
            TypeScope variableDeclarationScope = new TypeScope(variableDeclarations, parent);
            Visitor<Expression> variableDeclarationScopeTyper = generateTyper(variableDeclarationScope);
            iterationStatementDeclareFor.getVariableDeclaration().visitOwnedExpressions(variableDeclarationScopeTyper);
            iterationStatementDeclareFor.getCondition().visitNestedExpressions(variableDeclarationScopeTyper);
            iterationStatementDeclareFor.getIteration().visitNestedExpressions(variableDeclarationScopeTyper);
            TypeScope forBodyScope = getDeclarations(forBody, variableDeclarationScope);
            forBodyScope.putAll(variableDeclarations);
            functionScopes.put(forBody, forBodyScope);
            return forBodyScope;
        } else if (stm instanceof IterationStatementWhile) {
            IterationStatementWhile iterationStatementWhile = (IterationStatementWhile) stm;
            ((IterationStatementWhile) stm).getCondition().visitNestedExpressions(currentScopeVisitor);
            Statement whileBody = iterationStatementWhile.getStatement();
            TypeScope whileScope = getDeclarations(whileBody, parent);
            functionScopes.put(whileBody, whileScope);
            return whileScope;
        } else if (stm instanceof IterationStatementDoWhile) {
            Statement whileBody = ((IterationStatementDoWhile) stm).getStatement();
            ((IterationStatementDoWhile) stm).getCondition().visitNestedExpressions(currentScopeVisitor);
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
        } else if (stm instanceof ExpressionStatement) {
            stm.visitOwnedExpressions(currentScopeVisitor);
            return parent;
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
}
