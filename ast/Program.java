package ast;

import lombok.Value;

import java.util.Collection;

@Value
public class Program {
    private Collection<Function> function;
    private Collection<Declaration> declarations;
}