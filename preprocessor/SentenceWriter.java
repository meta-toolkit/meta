/**
 * @file SentenceWriter.java
 * Contains some simple preprocessing code in Java using the Stanford Parser.
 */

import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;
import java.io.FileWriter;
import java.io.IOException;
import edu.stanford.nlp.ling.HasWord;
import edu.stanford.nlp.parser.lexparser.LexicalizedParser;
import edu.stanford.nlp.process.DocumentPreprocessor;
import edu.stanford.nlp.trees.Tree;

class SentenceWriter
{
    /**
     * Preprocesses the document via the Stanford Parser's DocumentPreprocessor, finding sentence boundaries.
     * @param filename - The document to process
     * @return a list of sentences
     */
    private static List<List<? extends HasWord>> setupSentences(String filename)
    {
        DocumentPreprocessor dp = new DocumentPreprocessor(filename);
        List<List<? extends HasWord>> ret = new ArrayList<List<? extends HasWord>>();
        for(List<HasWord> sentence : dp)
            ret.add(sentence);
        return ret;
    }

    public static void main(String[] args) throws Exception
    {
        System.out.println("Processing " + args.length + " files.");
        int filesDone = 0;
        for(String filename: args)
        {
            FileWriter writer = new FileWriter(filename + ".sen");
            System.out.println("Parsing " + filename + " ... ");
            String numDone = "[" + (filesDone + 1) + "/" + args.length + "]: ";
            Iterable<List<? extends HasWord>> sentences = setupSentences(filename);
            for(List<? extends HasWord> sentence: sentences)
            {
                String words = "<s> ";
                for(HasWord word: sentence)
                    words += word.word() + " ";
                words += " </s>\n";
                writer.write(words);
            }
            writer.close();
            ++filesDone;
        }
    }
}
