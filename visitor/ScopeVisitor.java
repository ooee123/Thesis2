package visitor;

import ast.*;
import ast.declaration.VariableDeclaration;
import ast.statement.CanContainStatements;
import ast.statement.Statement;
import ast.statement.impl.CompoundStatement;
import ast.statement.impl.IterationStatementDeclareFor;
import ast.type.Type;
import lombok.Data;

import java.util.*;

/**
 * Created by ooee on 10/21/16.
 */
@Data
public class ScopeVisitor {
    private TypeScope globalScope;
    private Program program;
    private Map<BlockItem, Map<String, Type>> identityTypeMapper;

    public ScopeVisitor(Program program) {
        this.program = program;
        Map<String, Type> variableDeclarations = getVariableDeclarations(program.getVariableDeclarations().toArray(new VariableDeclaration[0]));
        globalScope = new TypeScope(variableDeclarations, null);
    }

    private Map<String, Type> getVariableDeclarations(VariableDeclaration... variableDeclarations) {
        Map<String, Type> declarationsMap = new HashMap<>();
        for (VariableDeclaration variableDeclaration : variableDeclarations) {
            for (VariableDeclaration.DeclaredVariable declaredVariable : variableDeclaration.getDeclaredVariables()) {
                String id = declaredVariable.getIdentifier().getIdentifier();
                Type type = declaredVariable.getType();
                declarationsMap.put(id, type);
            }
        }
        return declarationsMap;
    }

    public Map<BlockItem, Map<String, Type>> process(Function function) {
        identityTypeMapper = new IdentityHashMap<>();
        TypeScope functionScope = new TypeScope(globalScope);

        for (Parameter parameter : function.getParameterList().getParameters()) {
            String identifier = parameter.getFormalParameterName().getIdentifier();
            Type type = parameter.getType();
            functionScope.put(identifier, type);
        }
        visit(function.getCompoundStatement(), functionScope);
        identityTypeMapper.put(function.getCompoundStatement(), functionScope.flatten());
        return identityTypeMapper;
    }

    public void visit(CompoundStatement compoundStatement, TypeScope parent) {
        TypeScope scope = new TypeScope(parent);
        for (BlockItem blockItem : compoundStatement.getBlockItems()) {
            if (blockItem instanceof VariableDeclaration) {
                for (VariableDeclaration.DeclaredVariable declaredVariable : ((VariableDeclaration) blockItem).getDeclaredVariables()) {
                    String identifier = declaredVariable.getIdentifier().getIdentifier();
                    Type type = declaredVariable.getType();
                    scope.put(identifier, type);
                }
                identityTypeMapper.put(blockItem, scope.flatten());
            } else if (blockItem instanceof Statement) {
                visit(((Statement) blockItem), scope);
            }
        }
    }

    public void visit(IterationStatementDeclareFor statement, TypeScope parent) {
        TypeScope typeScope = new TypeScope(parent);
        for (VariableDeclaration.DeclaredVariable declaredVariable : statement.getVariableDeclaration().getDeclaredVariables()) {
            String identifier = declaredVariable.getIdentifier().getIdentifier();
            Type type = declaredVariable.getType();
            typeScope.put(identifier, type);
        }
        identityTypeMapper.put(statement, typeScope.flatten());
        visit(statement.getStatement(), typeScope);
    }

    public void visit(Statement statement, TypeScope parent) {
        if (statement instanceof IterationStatementDeclareFor) {
            visit(((IterationStatementDeclareFor) statement), parent);
        } else if (statement instanceof CompoundStatement) {
            visit(((CompoundStatement) statement), parent);
        } else if (statement instanceof CanContainStatements) {
            for (Statement statement1 : ((CanContainStatements) statement).getStatementNodes()) {
                visit(statement1, parent);
                identityTypeMapper.put(statement1, parent.flatten());
            }
        }
        identityTypeMapper.put(statement, parent.flatten());
    }
}
