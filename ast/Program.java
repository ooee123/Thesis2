package ast;

import lombok.Value;

import java.util.Collection;
import java.util.List;

@Value
public class Program implements BaseElement {
    private Collection<Function> function;
    private Collection<Declaration> declarations;

    @Override
    public String toCode() {
        List<String> functionCodes = collectionToCode(function);
        List<String> declarationCodes = collectionToCode(declarations);

        return String.join("\n", declarationCodes) + "\n\n" + String.join("\n", functionCodes);
    }
}