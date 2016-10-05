package visitor;

import ast.*;
import ast.type.Type;
import lombok.Value;

import java.util.*;

import static ast.Declaration.DeclaredVariable;

@Value
public class ASTGeneralVisitor {

    private Map<String, TypeScope> functionScopes;

    public ASTGeneralVisitor(Program p) {
        functionScopes = new LinkedHashMap<>();

        Map<String, Type> types = new LinkedHashMap<>();
        putDeclarations(p.getDeclarations(), types);
        TypeScope globalScope = new TypeScope(types, null);
        Collection<Function> functions = p.getFunction();
        for (Function f : functions) {
            String functionId = f.getIdentifier();
            Type functionReturnType = f.getReturnType();
            globalScope.put(functionId, functionReturnType);
            Map<String, Type> functionScope = getDeclarations(f.getCompoundStatement(), new HashMap<>());
            TypeScope functionTypeScope = new TypeScope(functionScope, globalScope);
            functionScopes.put(f.getIdentifier(), functionTypeScope);
        }
    }

    private Map<String, Type> getDeclarations(Statement stm, Map<String, Type> types) {
        if (stm instanceof SelectionStatementIf) {
            getDeclarations(((SelectionStatementIf) stm).getElseStatement(), types);
            getDeclarations(((SelectionStatementIf) stm).getThenStatement(), types);
        } else if (stm instanceof IterationStatementFor) {
            getDeclarations(((IterationStatementFor) stm).getStatement(), types);
        } else if (stm instanceof IterationStatementDeclareFor) {
            getDeclarations(((IterationStatementDeclareFor) stm).getStatement(), types);
            putDeclarations(Arrays.asList(((IterationStatementDeclareFor) stm).getDeclaration()), types);
        } else if (stm instanceof IterationStatementWhile) {
            getDeclarations(((IterationStatementWhile) stm).getStatement(), types);
        } else if (stm instanceof IterationStatementDoWhile) {
            getDeclarations(((IterationStatementDoWhile) stm).getStatement(), types);
        } else if (stm instanceof CompoundStatement) {
            Collection<BlockItem> blockItems = ((CompoundStatement) stm).getBlockItems();
            Collection<Declaration> declarations = new ArrayList<>();
            for (BlockItem blockItem : blockItems) {
                if (blockItem instanceof Declaration) {
                    declarations.add(((Declaration) blockItem));
                } else if (blockItem instanceof Statement) {
                    getDeclarations(((Statement) blockItem), types);
                }
            }
            putDeclarations(declarations, types);
        }
        return types;
    }

    private void putDeclarations(Collection<Declaration> declarations, Map<String, Type> types) {
        for (Declaration declaration : declarations) {
            for (DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
                String id = declaredVariable.getIdentifier();
                Type type = declaredVariable.getType();
                types.put(id, type);
            }
        }
    }
}
