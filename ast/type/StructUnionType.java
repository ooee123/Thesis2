package ast.type;

import ast.expression.impl.PrimaryExpressionIdentifier;
import ast.VariableDeclaration;
import lombok.*;

import java.util.List;

/**
 * Created by ooee on 9/24/16.
 */
@Data
@EqualsAndHashCode(exclude = {"declarations"})
public class StructUnionType implements ActualType {


    public enum StructUnion {
        STRUCT("struct"),
        UNION("union");

        private String token;

        StructUnion(String token) {
            this.token = token;
        }

        public static StructUnion toStructUnion(String token) {
            for (StructUnion structUnion : StructUnion.values()) {
                if (structUnion.token.equals(token)) {
                    return structUnion;
                }
            }
            throw new IllegalArgumentException("Illegal token of " + token);
        }
    }

    public StructUnionType(String tag, @NonNull StructUnion structUnion, List<VariableDeclaration> declarations) {
        this.tag = tag;
        this.structUnion = structUnion;
        this.declarations = declarations;
        this.typedefName = null;
        this.wasToCoded = false;
    }

    private boolean wasToCoded;
    private final String tag;
    @NonNull private final StructUnion structUnion;
    private final List<VariableDeclaration> declarations;
    @Setter private String typedefName;

    public String expandedStructUnion() {
        StringBuilder builder = new StringBuilder();
        builder.append(structUnion.token);
        if (tag != null) {
            builder.append(" " + tag);
        }
        if (declarations != null) {
            if (!wasToCoded) {
                wasToCoded = true;
                builder.append(" {\n");
                for (VariableDeclaration declaration : declarations) {
                    builder.append(declaration.toCode() + "\n");
                }
                builder.append("}");
            }
        }
        return builder.toString();
    }

    @Override
    public String toCode() {
        if (typedefName != null) {
            return typedefName;
        } else if (tag != null && wasToCoded) {
            return structUnion.token + " " + tag;
        } else {
            return expandedStructUnion();
        }
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("StructUnionType=(");
        builder.append("tag=" + tag);
        builder.append(", structUnion=" + structUnion);
        builder.append("declarations=");
        if (declarations != null) {
            builder.append("{");
            for (VariableDeclaration declaration : declarations) {
                for (VariableDeclaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
                    String variableType = declaredVariable.getType().toCode();
                    String identifier = declaredVariable.getIdentifier().getIdentifier();
                    builder.append(variableType + ":" + identifier + ",");
                }
            }
            builder.append("}");
        } else {
            builder.append("null");
        }
        builder.append(")");
        return builder.toString();
    }

    public Type getField(String fieldName) {
        for (VariableDeclaration declaration : declarations) {
            for (VariableDeclaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
                if (declaredVariable.getIdentifier().getIdentifier().equals(fieldName)) {
                    return declaredVariable.getType();
                }
            }
        }
        throw new IllegalArgumentException("Field name of " + fieldName + " not found in " + toString());
    }
}
