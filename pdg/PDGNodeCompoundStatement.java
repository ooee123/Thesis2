package pdg;

import ast.BlockItem;
import ast.CompoundStatement;
import ast.Statement;
import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeCompoundStatement extends PDGNode<CompoundStatement> {
    private Collection<PDGNode> body;

    public PDGNodeCompoundStatement(CompoundStatement compoundStatement, Collection<PDGNode> body) {
        this(compoundStatement, body, false);
    }

    public PDGNodeCompoundStatement(CompoundStatement compoundStatement, Collection<PDGNode> body, boolean required) {
        super(compoundStatement, required);
        this.body = body;
    }

    @Override
    public CompoundStatement sort(PDGSorter sorter) {
        Statement sort = sorter.sort(body);
        if (body.size() > 1) {
            return ((CompoundStatement) sort);
        } else {
            return new CompoundStatement(Lists.newArrayList(sort));
        }
    }
}
