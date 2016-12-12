package ast;

import ast.declaration.TypedefDeclaration;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

@Value
public class Program implements BaseElement {
    private Collection<Function> function;
    private Collection<Declaration> declarations;

    @Override
    public String toCode() {
        List<String> declarationCodes = collectionToCode(declarations);
        List<String> functionCodes = collectionToCode(function);

        return String.join("\n", declarationCodes) + "\n\n" + String.join("\n", functionCodes);
    }

    public Collection<VariableDeclaration> getVariableDeclarations() {
        Collection<VariableDeclaration> variableDeclarations = new ArrayList<>();
        for (Declaration declaration : declarations) {
            if (declaration instanceof VariableDeclaration) {
                variableDeclarations.add(((VariableDeclaration) declaration));
            }
        }
        return variableDeclarations;
    }

    public Collection<TypedefDeclaration> getTypedefDeclarations() {
        Collection<TypedefDeclaration> typedefDeclarations = new ArrayList<>();
        for (Declaration declaration : declarations) {
            if (declaration instanceof TypedefDeclaration) {
                typedefDeclarations.add(((TypedefDeclaration) declaration));
            }
        }
        return typedefDeclarations;
    }
}